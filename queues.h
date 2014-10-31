/*
 * queues.h
 *
 *  Created on: Oct 26, 2014
 *  Author: Ahmed Talaat (aa_talaat@yahoo.com) - based on an implementation from the Internet
 */
#ifndef QUEUES_H_
#define QUEUES_H_

#define MAX_ITEMS    20

/*
 * Type of messages we will deal with
 */
typedef enum {	MSG_BT_DOWN, MSG_BT_UP } MSGID;

typedef	struct						// queue element content
{
	  MSGID msgID;
	  uint8_t msgContent;
} msgQueueDef;

typedef struct circularQueue_s {
    uint8_t     first;
    uint8_t     last;
    uint8_t     validItems;
    msgQueueDef	data[MAX_ITEMS];
} circularQueue_t;

extern circularQueue_t   IsrToMainQueue;

void initializeQueue(circularQueue_t *theQueue);
uint8_t isEmpty(circularQueue_t *theQueue);
uint8_t putItemInQueue(circularQueue_t *theQueue, msgQueueDef *theItemValue);
uint8_t getItemFromQueue(circularQueue_t *theQueue, msgQueueDef *theItemValue);

#endif /* QUEUES_H_ */
