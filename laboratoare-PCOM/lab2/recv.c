#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "link_emulator/lib.h"
#include "include/utils.h"

/**
 * You can change these to communicate with another colleague.
 * There are several factors that could stop this from working over the
 * internet, but if you're on the same network it should work.
 * Just fill in their IP here and make sure that you use the same port.
 */
#define HOST "127.0.0.1"
#define PORT 10001

#include "common.h"

/* Our unique layer 2 ID */
static int ID = 123131;

/* Function which our protocol implementation will provide to the upper layer. */
int recv_frame(char *buf, int size)
{
	/* TODO 1.1: Call recv_byte() until we receive the frame start
	 * delimitator. This operation makes this function blocking until it
	 * receives a frame. */

	/* TODO 2.1: The first two 2 * sizeof(int) bytes represent sender and receiver ID */

	/* TODO 2.2: Check that the frame was sent to me */

	/* TODO 1.2: Read bytes and copy them to buff until we receive the end of the frame */

	/* If everything went well return the number of bytes received */
	return 0;
}

int main(int argc,char** argv){
	/* Don't modify this */
	init(HOST,PORT);

        // TODO remove these recives, whih are hardcoded to receive a "Hello"
	// message, and replace them with code that can receive any message.
	char c;

	/* Wait for the start of a frame */
	char c1,c2;
	c1 = recv_byte();
	c2 = recv_byte();

	/* Cat timp nu am primit DLE STX citim bytes */
	while((c1 != DLE) && (c2 != STX)) {
		c1 = c2;
		c2 = recv_byte();
	}


	printf("%d ## %d\n",c1, c2);
	c = recv_byte();
	printf("%c\n", c);

	c = recv_byte();
	printf("%c\n", c);

	c = recv_byte();
	printf("%c\n", c);

	c = recv_byte();
	printf("%c\n", c);

	c = recv_byte();
	printf("%c\n", c);

	c = recv_byte();
	printf("%c\n", c);

	/* TODO 1.0: Allocate a buffer and call recv_frame */


	/* TODO 3: Measure latency in a while loop for any frame that contains
	 * a timestamp we receive, print frame_size and latency */

	printf("[RECEIVER] Finished transmission\n");
	return 0;
}
