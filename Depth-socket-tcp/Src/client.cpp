/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <pthread.h>
#include <unistd.h>
#include <OpenNI.h>

#include "OniSampleUtilities.h"

#define SAMPLE_READ_WAIT_TIMEOUT    2000 //2000ms
#define MYPORT                      6666
#define BUFFER_SIZE                 1024
#define DEPTH_WIDTH                 320
#define DEPTH_HEIGHT                240
#define POINTER_NUM                 (DEPTH_WIDTH*DEPTH_HEIGHT)

using namespace openni;

typedef struct
{
    int index;
    int depth[POINTER_NUM];
}Node;

Node *myNode = (Node*)malloc(sizeof(Node));

int send_depth()
{
    ///sockfd
    int sock_cli = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(MYPORT);
    servaddr.sin_addr.s_addr = inet_addr("192.168.62.130");
    //servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //servaddr.sin_addr.s_addr = inet_addr("192.168.1.108");


    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect");
        exit(1);
    }

    int needSend = sizeof(Node);
    char *buffer = (char*)malloc(needSend);
    memcpy(buffer,myNode,needSend);

    int pos = 0;
    int len = 0;
    while(pos < needSend)
    {
        len = send(sock_cli, buffer+pos, BUFFER_SIZE, 0);
        if(len <= 0)
        {
            perror("ERROR");
            break;
        }
        pos+=len;
    }

    buffer = NULL;
    //myNode = NULL;
    free(buffer);
    //free(myNode);

    close(sock_cli);
    printf(">>>>>>>>>>Send info test>>>>>>>>>>\n");
    printf("index:%d\n"
    		"depth[5000] :%5d depth[10000]:%5d depth[15000]:%5d depth[20000]:%5d\n"
    		"depth[25000]:%5d depth[30000]:%5d depth[35000]:%5d depth[40000]:%5d\n", 
    		myNode->index,
    		myNode->depth[5000],  myNode->depth[10000], myNode->depth[15000], myNode->depth[20000],
    		myNode->depth[25000], myNode->depth[30000], myNode->depth[35000], myNode->depth[40000]);

    return 0;
}

int main(int argc, char* argv[])
{
	Status rc = OpenNI::initialize();
	if (rc != STATUS_OK)
	{
		printf("Initialize failed\n%s\n", OpenNI::getExtendedError());
		return 1;
	}

	Device device;

    if (argc < 2)
        rc = device.open(ANY_DEVICE);
    else
        rc = device.open(argv[1]);

    if (rc != STATUS_OK)
	{
		printf("Couldn't open device\n%s\n", OpenNI::getExtendedError());
		return 2;
	}

	VideoStream depth;

	if (device.getSensorInfo(SENSOR_DEPTH) != NULL)
	{
		rc = depth.create(device, SENSOR_DEPTH);
		if (rc != STATUS_OK)
		{
			printf("Couldn't create depth stream\n%s\n", OpenNI::getExtendedError());
			return 3;
		}
	}

	VideoMode depthVideoMode = depth.getVideoMode();
	depthVideoMode.setResolution(DEPTH_WIDTH, DEPTH_HEIGHT);
	depthVideoMode.setFps(30);
	depth.setVideoMode(depthVideoMode);

	rc = depth.start();
	if (rc != STATUS_OK)
	{
		printf("Couldn't start the depth stream\n%s\n", OpenNI::getExtendedError());
		return 4;
	}

	VideoFrameRef frame;

	//while (!wasKeyboardHit())
	printf("---------------------Hotkey----------------------\n");
	printf("1.Press s to send one frame depth data.\n");
	printf("2.Press q to quit.\n");
	while (1)
	{
		int changedStreamDummy;
		VideoStream* pStream = &depth;
		rc = OpenNI::waitForAnyStream(&pStream, 1, &changedStreamDummy, SAMPLE_READ_WAIT_TIMEOUT);
		if (rc != STATUS_OK)
		{
			printf("Wait failed! (timeout is %d ms)\n%s\n", SAMPLE_READ_WAIT_TIMEOUT, OpenNI::getExtendedError());
			continue;
		}

		rc = depth.readFrame(&frame);
		if (rc != STATUS_OK)
		{
			printf("Read failed!\n%s\n", OpenNI::getExtendedError());
			continue;
		}

		if (frame.getVideoMode().getPixelFormat() != PIXEL_FORMAT_DEPTH_1_MM && frame.getVideoMode().getPixelFormat() != PIXEL_FORMAT_DEPTH_100_UM)
		{
			printf("Unexpected frame format\n");
			continue;
		}

		DepthPixel* pDepth = (DepthPixel*)frame.getData();
		myNode->index = frame.getFrameIndex();
		for (int i = 0; i < POINTER_NUM; i++)
		{
			myNode->depth[i] = pDepth[i];
		}
		
		char c = getchar();
		if (c == 's')
		{
			send_depth();
		} 
		else if (c == 'q')
		{
			break;
		} 
		//printf("[%d], [%d], [%d], [%d] !!!\n", myNode->index, myNode->depth[9680], frame.getWidth(), frame.getHeight());
	}

	myNode = NULL;
	free(myNode);
	depth.stop();
	depth.destroy();
	device.close();
	OpenNI::shutdown();

	return 0;
}
