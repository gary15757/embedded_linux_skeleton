/*
 * utilities.cpp
 *
 *  Created on: Jul 26, 2018
 *      Author: hhdang
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <syslog.h>

#include "utilities.h"

#define BUF_SIZE 1024

void write_pid(const char*pidfile, pid_t pid)
{
    FILE *fp;

    if ((fp = fopen(pidfile, "w")) == NULL) {
        fprintf(stderr, "Warning: can't write PID file %s.\n", pidfile);
        return;
    }

    fprintf(fp, "%d\n", (int)pid);
    fclose(fp);

    if (chmod(pidfile, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) == -1) {
        fprintf(stderr, "Warning: can't chmod PID file %s. Make sure it's only writable for root!\n", pidfile);
    }
}

bool copy_file(const char *src, const char*dst)
{
    int     src_fd, dst_fd;
    ssize_t num_read;
    char    buf[BUF_SIZE];
    bool    rc = true;

    src_fd = open(src, O_RDONLY);

    dst_fd = creat(dst, S_IRUSR | S_IWUSR);

    if (src_fd != -1 && dst_fd != -1) {
        while ((num_read = ::read(src_fd, buf, BUF_SIZE)) > 0) {

            if (::write(dst_fd, buf, num_read) != num_read) {
                syslog(LOG_ERR, "%s-%d: couldn't write whole buffer\n", __FUNCTION__, __LINE__);
                rc = false;
                goto out;
            }

        }

        if (num_read == -1) {
            syslog(LOG_ERR, "%s-%d: read failed\n", __FUNCTION__, __LINE__);
            rc = false;
            goto out;
        }
    }

out:
    if (src_fd != -1)
        close(src_fd);

    if (dst_fd != -1)
        close(dst_fd);

    return rc;
}
