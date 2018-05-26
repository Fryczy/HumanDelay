#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h> // needed for memset

int serialHandleAndPlot(int pipe) {

	struct termios tio;
	struct termios stdio;
	int tty_fd;
	//int i = 0;
	//int MsgCount = 0;
	int DataToPlot;

	const char *cmd ="plot '/home/student/workspace/UARTRead/data.txt' with points pointtype 5\n";
	const char *plotSettingsCmd ="set yrange [-10000:100000] \n set xrange [0:20] \n";
	unsigned char c = 'D';
	printf("Jöhetnek az adatok\n");
	memset(&stdio, 0, sizeof(stdio));
	stdio.c_iflag = 0;
	stdio.c_oflag = 0;
	stdio.c_cflag = 0;
	stdio.c_lflag = 0;
	stdio.c_cc[VMIN] = 1;
	stdio.c_cc[VTIME] = 0;
	tcsetattr(STDOUT_FILENO, TCSANOW, &stdio);
	tcsetattr(STDOUT_FILENO, TCSAFLUSH, &stdio);

	memset(&tio, 0, sizeof(tio));
	tio.c_iflag = 0;
	tio.c_oflag = 0;
	tio.c_cflag = CS8 | CREAD | CLOCAL; // 8n1, see termios.h for more information
	tio.c_lflag = 0;
	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 5;
	tty_fd = open("/dev/ttyACM0", O_RDONLY);
	if (tty_fd == -1) {
		perror("open tty");
	}
	cfsetospeed(&tio, B115200);            // 115200 baud
	cfsetispeed(&tio, B115200);            // 115200 baud

	tcsetattr(tty_fd, TCSANOW, &tio);

	DataToPlot = open("data.txt", O_RDWR | O_CREAT, S_IRWXU); // A soros porton érkező fájok tárolásra szolgáló fájl

	if (DataToPlot == -1) {
		perror("open");
	}
	write(pipe,plotSettingsCmd,strlen(plotSettingsCmd));

	while (c!='q') {
		if (read(tty_fd, &c, 1) > 0) {
			write(STDOUT_FILENO, &c, 1); // if new data is available on the serial port, print it out
			write(DataToPlot, &c, 1); // kiiratjuk a data.txt-be melyek később kirajzoltatunk

		}
		//MsgCount++;
		write(pipe, cmd, strlen(cmd));
	}

	close(tty_fd);

	return 0;
}

int main(int argc, char * argv[], char * env[]) {

	int csovek[2];

	pipe(csovek);

	if (fork() == 0) {

		// gyerek

		char * arg[] = { "/usr/bin/gnuplot", NULL };

		// stdin bezár

		close(0);

		// stdin helyett

		dup(csovek[0]); //  read ág

		// ez lesz az stdin helyén

		execve("/usr/bin/gnuplot", arg, env);

	}

	else {
		int i=0;
		while(1){
			for (i = 0; i < 10; ++i) {
				serialHandleAndPlot(csovek [1]);
			}

		}
	}

	return 0;

}

