// OLD CODE from serial port projects
// released MIT-0

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

int readEchoLoop(int fd);
int readPort(int fd, int ms);

union {
    uint16_t word;
    struct {
        uint8_t byte1;
        uint8_t byte2;
    };
} u1;

int set_interface_attribs(int fd, int speed) {
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD); /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;      /* 8-bit characters */
    tty.c_cflag &= ~PARENB;  /* no parity bit */
    tty.c_cflag &= ~CSTOPB;  /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS; /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

void set_mincount(int fd, int mcount) {
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error tcgetattr: %s\n", strerror(errno));
        return;
    }

    tty.c_cc[VMIN] = mcount ? 1 : 0;
    tty.c_cc[VTIME] = 5; /* half second timer */

    if (tcsetattr(fd, TCSANOW, &tty) < 0) printf("Error tcsetattr: %s\n", strerror(errno));
}

int main(int argc, char *argv[]) {
    char *portname;
    int loop = 0;
   
    int fd;
    int slen, wlen;

    if (argc < 2) {
        printf("Usage: %s <port> [echo]\n", argv[0]);
        return -1;
    }
    portname = argv[1];

    if (argc > 2 && strncmp(argv[2], "echo", strlen("echo")) == 0) {
        loop = 1;
        printf("Looping on %s\n", portname);
    }
    fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        printf("Error opening %s: %s\n", portname, strerror(errno));
        return -1;
    }
    /*baudrate 9600, 8 bits, no parity, 1 stop bit */
    set_interface_attribs(fd, B9600);
    // set_mincount(fd, 0); /* set to pure timed read */

    if (loop) {
        readEchoLoop(fd);
    }
    unsigned char buffer[256];

    buffer[0] = 0xFF;
    buffer[1] = 0x00;
    buffer[2] = 0xFF;
    buffer[3] = 0xA5;
    buffer[4] = 0x00;
    buffer[5] = 0x60;
    buffer[6] = 0x10;
    buffer[7] = 0x04;
    buffer[8] = 0x01;
    buffer[9] = 0xFF;  
    buffer[10] = 0x02;
    buffer[11] = 0x19;
    

//    buffer[7] = 0x07;
//    buffer[8] = 0x00;
//    buffer[9] = 0x00;   // checksum
//    buffer[10] = 0x00;  // checksum
    slen = 12;
    u1.word = buffer[3] + buffer[4] + buffer[5] + buffer[6] + buffer[7] + buffer[8];
    // u1.word = (u1.word%256)
//    printf("Checksum: %04X\n", u1.word);
//    printf("or: %02X %02X\n", u1.byte1, u1.byte2);
//    fflush(stdout);
//    buffer[9] = u1.byte2;
//    buffer[10] = u1.byte1;

    for (int x = 0; x < slen; x++) {
        printf("%02X ", (unsigned int)buffer[x]);
    }
    printf("\n");

//    while(1) {
    wlen = write(fd, buffer, slen);
    if (wlen != slen) {
        printf("Error from write: %d, %d\n", wlen, errno);
    }
    tcdrain(fd); /* delay for output */
    printf("wrote %d bytes\n", slen);
//    }
    // readEchoLoop(fd);
    readPort(fd, 2000);
}

int readPort(int fd, int ms) {
    struct timeval tv;
    fd_set rset;
    int retV;
    int timeout = ms;  // 5 seconds
    unsigned char buf[256];
    int rdlen = 0;
    memset(buf, 0x00, 256);
    tv.tv_usec = (timeout * 1000) % 1000000;
    tv.tv_sec = timeout / 1000;

    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    retV = select(fd + 1, &rset, NULL, NULL, &tv);

    if (retV == 0) {
        // timeout stuff
        printf("Timeout\n");
    } else if (retV < 0) {
        printf("Error\n");  // Error stuff. Read errno to see what happened
    } else {
        rdlen = read(fd, buf, 1);
        printf("read %d\n", rdlen);
        if (rdlen > 0) {
            for (int x = 0; x < rdlen; x++) {
                printf("%02x ", (unsigned int)buf[x]);
                fflush(stdout);
            }
        }
    }
}

int readEchoLoop(int fd) {
    printf("reading...\n");
    do {
        printf(".");
        fflush(stdout);
        unsigned char buf[256];
        int rdlen = 0;
        memset(buf, 0x00, 256);

        rdlen = read(fd, buf, 1);
        printf("read %d\n", rdlen);
        if (rdlen > 0) {
            for (int x = 0; x < rdlen; x++) {
                printf("%02x ", (unsigned int)buf[x]);
                fflush(stdout);
            }
        } else if (rdlen < 0) {
            printf("Error from read: %d: %s\n", rdlen, strerror(errno));
        }
        /* repeat read to get full message */
    } while (1);
}
