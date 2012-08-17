/*
 * ServiceDirectoryTestSuite.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Mark Barger
 */

#ifndef SERVICEDIRECTORYTESTSUITE_H_
#define SERVICEDIRECTORYTESTSUITE_H_

#include <stdlib.h>
#include <iostream>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include "zmq.h"
#include "cxxtest/TestSuite.h"
#include "DataProductDescriptionPB.pb.h"

using namespace std;
using namespace gravity;

#define READ 0
#define WRITE 1
#define BUFFER_SIZE 1000

pid_t popen2(const char *command, int *infp, int *outfp) {
	int p_stdin[2], p_stdout[2];
	pid_t pid;

	if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
		return -1;
	pid = fork();
	if (pid < 0)
		return pid;
	else if (pid == 0) {
		close(p_stdin[WRITE]);
		dup2(p_stdin[READ], READ);
		close(p_stdout[READ]);
		dup2(p_stdout[WRITE], WRITE);
		execl("/bin/sh", "sh", "-c", command, (char *)NULL);
		perror("execl");
		exit(1);
	}

	if (infp == NULL)
		close(p_stdin[WRITE]);
	else
		*infp = p_stdin[WRITE];
	if (outfp == NULL)
		close(p_stdout[READ]);
	else
		*outfp = p_stdout[READ];

	close(p_stdin[READ]);
	close(p_stdout[WRITE]);

	return pid;
}

class ServiceDirectoryTestSuite: public CxxTest::TestSuite {

public:
	void setUp() {
//		context = zmq_init(1);

		memset(buffer, 0, BUFFER_SIZE);
		pid = popen2("ServiceDirectory", NULL, &sdFd);
		int nbytes = read(sdFd, buffer, BUFFER_SIZE);
		cout << endl << "output: " << buffer << endl;
//		struct timeval tv = { 0,  1000000L};
//		select(0, NULL, NULL, NULL, &tv );
	}

	void tearDown() {
		popen2("pkill -f \"^ServiceDirectory$\"", NULL, NULL);
//		if (pid > 0) {
//			kill(pid, SIGTERM);
//		}
	}

	void testRegister(void) {
//		void *socket = zmq_socket(context, ZMQ_REQ);

		DataProductDescriptionPB dpd;
		dpd.set_dataproductid("TestDataProduct1");
		dpd.set_url("TestUrl1");

//		int nbytes = read(sdFd, buffer, BUFFER_SIZE);
//		cout << endl << "output: " << buffer << endl;

	}

private:
	pid_t pid;
	int sdFd;
	char buffer[BUFFER_SIZE];
	void *context;
};

#endif /* SERVICEDIRECTORYTESTSUITE_H_ */
