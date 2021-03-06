/*
 * Copyright (C) 2007, 2008, 2009
 *       pancake <youterm.com>
 *
 * radare is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * radare is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with radare; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <plugin.h>
#include <utils.h>
#include <socket.h>
#if __UNIX__
#include <netinet/in.h>
#include <signal.h>
#endif
#include <string.h>

enum {
	REMOTE_NONE=0,
	REMOTE_CLIENT=1,
	REMOTE_LISTEN=2
};

int remote_mode = REMOTE_NONE;
int remote_fd = 0;

#define RMT_OPEN   0x01
#define RMT_READ   0x02
#define RMT_WRITE  0x03
#define RMT_SEEK   0x04
#define RMT_CLOSE  0x05
#define RMT_SYSTEM 0x06
#define RMT_CMD    0x07
#define RMT_REPLY  0x80

#define uchar unsigned char

ssize_t remote_write(int fd, const void *buf, size_t count)
{
	unsigned char *tmp;
	unsigned int size = (int)count;
	int ret;

	tmp = (unsigned char *)malloc(count+5);

	tmp[0] = RMT_WRITE;
	endian_memcpy((uchar *)tmp+1, (uchar *)&size, 4);
	memcpy(tmp+5, buf, size);

	ret = write(remote_fd, tmp, size+5);

	// recv
	read(remote_fd, tmp, 5);

	free(tmp);
	// TODO: get reply

        return ret;
}

ssize_t remote_read(int fd, void *buf, size_t count)
{
	uchar tmp[5];
	int i = (int)count;

	// send
	tmp[0] = RMT_READ;
	endian_memcpy(tmp+1, (uchar*)&i, 4);
	write(remote_fd, tmp, 5);

	// recv
	read(remote_fd, tmp, 5);
	if (tmp[0] != (RMT_READ|RMT_REPLY)) {
		printf("Unexpected remote read reply (0x%02x)\n", tmp[0]);
		return -1;
	}
	endian_memcpy((uchar*)&i, tmp+1, 4);
	read(remote_fd, buf, i);

        return i; 
}

int remote_handle_client( int fd ){
	uchar buf[1024];
	uchar cmd;
	uchar flg;
	uchar *ptr;
	int i, c;
	ut64 x;

	if (fd == -1) {
		printf("Cannot listen.\n");
		return -1;
	}

	while ((c = accept(fd, NULL, NULL))) {
		if (c == -1) {
			printf("Cannot accept\n");
			close(c);
			return -1;
		}

		printf("Client connected\n");
		
		while(1) {
			i = read(c, &cmd, 1);
			if (i==0) {
				eprintf("Broken pipe\n");
				return -1;
			}

			switch((unsigned char)cmd) {
			case RMT_OPEN:
				read(c, &flg, 1); // flags
				printf("open (%d): ", cmd); fflush(stdout);
				read(c, &cmd, 1); // len
				ptr = malloc(cmd);
				if (ptr == NULL) {
					eprintf("Cannot malloc in rmt-open len = %d\n", cmd);
					return -1;
				}
				read(c, ptr, cmd); //filename
				memcpy(config.file, ptr, cmd);
				config.file[cmd]='\0';
				eprintf("(flags: %hhd) len: %hhd filename: '%s'\n",
					flg, cmd, config.file);
				buf[0] = RMT_OPEN | RMT_REPLY;
				config.fd = -1;
				radare_open(0);
				endian_memcpy(buf+1, (uchar *)&config.fd, 4);
				write(c, buf, 5);
				free(ptr);
				break;
			case RMT_READ:
				read(c, &buf, 4);
				endian_memcpy((uchar*)&i, buf, 4);
				ptr = (uchar *)malloc(config.block_size+i+5);
				radare_read(0);
				ptr[0] = RMT_READ|RMT_REPLY;
				if (i>config.block_size) i = config.block_size;
				endian_memcpy(ptr+1, (uchar *)&i, 4);
				memcpy(ptr+5, config.block, config.block_size);
				write(c, ptr, i+5);
				break;
			case RMT_CMD:
				{
				char bufr[8], *bufw = NULL;
				char *cmd = NULL, *cmd_output = NULL;
				int i, cmd_len = 0;

				/* read */
				read(c, &bufr, 4);
				endian_memcpy((uchar*)&i, bufr, 4);
				if (i <1) {
					eprintf("Invalid length '%d'\n", i);
				} else {
					cmd = malloc(i);
					read(c, cmd, i);
					cmd[i] = '\0';
					printf("len: %d cmd: '%s'\n",
						i, cmd); fflush(stdout);
					cmd_output = radare_cmd_str(cmd);
					if (cmd_output) 
						cmd_len = strlen(cmd_output) + 1;
					free(cmd);
					/* write */
					bufw = malloc(cmd_len + 5);
					bufw[0] = RMT_CMD | RMT_REPLY;
					endian_memcpy(bufw+1, (uchar *)&cmd_len, 4);
					memcpy(bufw+5, cmd_output, cmd_len);
					write(c, bufw, cmd_len+5);
					free(bufw);
					free(cmd_output);
				}
				break;
				}
			case RMT_WRITE:
				read(c, buf, 5);
				endian_memcpy((uchar *)&x, buf+1, 4);
				ptr = malloc(x);
				read(c, ptr, x);
				radare_write_at(config.seek, ptr, x);
				free(ptr);
				break;
			case RMT_SEEK:
				read(c, buf, 9);
				endian_memcpy((uchar *)&x, buf+1, 8);
				config.seek = io_lseek(config.fd, x, buf[0]);
				x = (int)config.seek;
				buf[0] = RMT_SEEK | RMT_REPLY;
				endian_memcpy(buf+1, (uchar*)&x, 8);
				write(c, buf, 9);
				break;
			case RMT_CLOSE:
				// XXX : proper shutdown
				read(c, buf, 4);
				endian_memcpy((uchar*)&i, buf, 4);
				{
				int ret = close(i);
				endian_memcpy(buf+1, (uchar*)&ret, 4);
				buf[0] = RMT_CLOSE | RMT_REPLY;
				write(c, buf, 5);
				}
				break;
			case RMT_SYSTEM:
				// read
				read(c, buf, 4);
				endian_memcpy((uchar*)&i, buf, 4);
				ptr = (uchar *) malloc(i+6);
				ptr[5]='!';
				read(c, ptr+6, i);
				ptr[6+i]='\0';
				//env_update();
				pipe_stdout_to_tmp_file((char*)&buf, (char*)ptr+5);
				{
					FILE *fd = fopen((char*)buf, "r");
					free(ptr); i = 0;
					if (fd == NULL) {
						printf("Cannot open tmpfile\n");
						i = -1;
					} else {
						fseek(fd, 0, SEEK_END);
						i = ftell(fd);
						fseek(fd, 0, SEEK_SET);
						ptr = (uchar *) malloc(i+5);
						fread(ptr+5, i, 1, fd);
						ptr[i+5]='\0';
						fclose(fd);
					}
				}
				unlink((char*)buf);

				// send
				ptr[0] = (RMT_SYSTEM | RMT_REPLY);
				endian_memcpy((uchar*)ptr+1, (uchar*)&i, 4);
				if (i<0)i=0;
				write(c, ptr, i+5);
				printf("REPLY SENT (%d) (%s)\n", i, ptr+5);
				free(ptr);
				break;
			default:
				printf("unknown command 0x%02x\n", cmd);
				close(c);
				return -1;
			}
		}
	}
	return 0;
}

int remote_open(const char *pathname, int flags, mode_t mode)
{
	int i;
	char *file;
	char buf[1024];

	strncpy(buf, pathname, 1000);
	if (!memcmp(buf, "rap://", 6)) {
		file = strchr(buf+6, ':');
		if (!file) {
			eprintf("No port defined.\n");
			return -1;
		}
		*file='\0';
		if (buf[6]=='\0')
			snprintf(buf, 1023, "listen://:%d", atoi(file+1));
		else snprintf(buf, 1023, "connect://%s:%d", pathname+6, atoi(file+1));
	}

	if (!memcmp(buf, "connect://", 10)) {
		// port
		char *port = strchr(buf+10, ':');
		if (port == NULL) {
			eprintf("No port defined.\n");
			return -1;
		}
		port[0] = '\0';

		// file
		file   = strchr(pathname+12,'/');
		if (file == NULL) {
			printf("No remote file specified.\n");
			return -1;
		}

		// connect
		remote_fd = socket_connect((char*)buf+10, atoi(port+1));
		if (remote_fd>=0)
			printf("Connected to: %s at port %d\n", buf+10, atoi(port+1));
		else {
			printf("Cannot coonect to '%s' (%d)\n", buf, atoi(port+1));
			return -1;
		}
		// send
		buf[0] = RMT_OPEN;
		buf[1] = flags;
		buf[2] = (uchar)strlen(file)-1;
		memcpy(buf+3, file+1, buf[2]);
		write(remote_fd, buf, 3+buf[2]);
		// read
		printf("waiting... "); fflush(stdout);
		read(remote_fd, buf, 5);
		if (buf[0] != (char)(RMT_OPEN|RMT_REPLY))
			return -1;

		endian_memcpy((uchar *)&i, (uchar*)buf+1, 4);
		if (i>0) printf("ok\n");
		config.fd = remote_fd;

		return remote_fd;
	}

	if (!memcmp(buf, "listen://", 9)) {
		char *port = strchr((char*)buf+9, ':');
		int p;
		if (port == NULL) {
			printf("No port defined.\n");
			return -1;
		}
		buf[0] = '\0';
		p = atoi(port+1);
		if (p<=0) {
			eprintf("Cannot listen here. Try listen://:9999\n");
			return -1;
		}
#if __UNIX__
		signal(SIGINT, exit); // ???
#endif
		printf("Listening at port %d\n", p);
		return remote_handle_client( socket_listen(p) );
	}

	printf("Oops\n");
	return -1;
}

int remote_close(int fd)
{
	return close(remote_fd);
}

ut64 remote_lseek(int fildes, ut64 offset, int whence)
{
	uchar tmp[10];

	// query
	tmp[0] = RMT_SEEK;
	tmp[1] = (uchar)whence;
	endian_memcpy(tmp+2, (uchar*)&offset, 8);
	write(remote_fd, &tmp, 10);

	// get reply
	read(fildes, &tmp, 9);
	if (tmp[0] != (RMT_SEEK | RMT_REPLY)) {
		printf("Unexpected lseek reply\n");
		return -1;
	}
	endian_memcpy((uchar *)&offset, tmp+1, 8);
	
	return offset;
}

int remote_handle_fd(int fd)
{
	return (fd == remote_fd);
}

int remote_handle_open(const char *file)
{
	if (!memcmp(file, "rap://", 6))
		return 1;
	if (!memcmp(file, "connect://", 10))
		return 1;
	if (!memcmp(file, "listen://", 9))
		return 1;
	return 0;
}

int remote_system(const char *command)
{
	uchar buf[1024];
	char *ptr;
	int i,j;

	if (command[0] == '!')
		return system(command+1);

	// send
	buf[0] = RMT_SYSTEM;
	i = strlen(command);
	endian_memcpy(buf+1, (uchar*)&i, 4);
	memcpy(buf+5, command, i);
	write(remote_fd, buf, i+5);

	// read
	read(remote_fd, buf, 5);
	if (buf[0] != (RMT_SYSTEM | RMT_REPLY)) {
		printf("Unexpected system reply\n");
		return -1;
	}
	endian_memcpy((uchar*)&i, buf+1, 4);
	if (i == -1) //0xFFFFFFFF) {
		return -1;
	ptr = (char *)malloc(i);
	read(remote_fd, ptr, i);
	j = write(1, ptr, i);
	free(ptr);
	return i-j;
}

plugin_t remote_plugin = {
	.name        = "remote",
	.desc        = "TCP IO ( listen://:port or connect://host:port )",
	.init        = NULL,
	.debug       = NULL,
	.system      = remote_system,
	.handle_fd   = remote_handle_fd,
	.handle_open = remote_handle_open,
	.open        = remote_open,
	.read        = remote_read,
	.write       = remote_write,
	.lseek       = remote_lseek,
	.close       = remote_close
};
