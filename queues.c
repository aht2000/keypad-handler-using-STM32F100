/*
 * queues.c
 *
 *  Created on: Oct 26, 2014
 *  Author: Ahmed Talaat (aa_talaat@yahoo.com) - based on an implementation from the Internet
 *
 * Contains all the functions required to create and manage a queue.
 *
 * The queue will store a maximum of MAX_ITEMS items, if we'll try to put more items than MAX_ITEMS, the new items will be lost.
 * To access the queue we will have 5 functions:
 *		- initialize
 *		- getItem
 *		- putItem
 *		- isEmpty
 * The initialize function clears the queue structure and it puts to zero both rear and front pointers.
 * The getItem function retrieves the next available item, the it moves the front pointer ahead by one element.
 * The putItem function puts a new item at the end of the queue and then it moves the rear pointer ahead by one element.
  * The isEmpty function returns true if rear and front pointers have the same values.
 */
#include "stm32f10x.h"
#include "queues.h"

circularQueue_t   IsrToMainQueue;

/*
 * the initializeQueue function, simply, sets to zero the structure
 */
void initializeQueue(circularQueue_t *theQueue) {
    uint8_t i;

    theQueue->validItems  =  0;
    theQueue->first       =  0;
    theQueue->last        =  0;

    for(i=0; i<MAX_ITEMS; i++) {
    	theQueue->data[i].msgID = 0;
    	theQueue->data[i].msgContent = 0;
    }
    return;
}

/*
 *  The isEmpty function verifies if the queue is empty (1) or if it contains some elements (0)
 */
uint8_t isEmpty(circularQueue_t *theQueue) {

    if(theQueue->validItems==0)
        return(1);
    else
        return(0);
}

/*
 * The putItem function, verifies if there is space in the queue, and returns 0xFF if the queue is full or add an item
 * at the end of the queue (theQueue->last element). Then it updates the value of theQueue->last the modulus operator
 * is needed to stay into the boundaries of the array and return 1.
 */
uint8_t putItemInQueue(circularQueue_t *theQueue, msgQueueDef *theItemValue) {

    if(theQueue->validItems>=MAX_ITEMS) {
        return(0xFF);
    } else {
        theQueue->validItems++;
        theQueue->data[theQueue->last].msgID = theItemValue->msgID;
        theQueue->data[theQueue->last].msgContent = theItemValue->msgContent;
        theQueue->last = (theQueue->last+1)%MAX_ITEMS;
        return(1);
    }
}

/*
 * The getItem function returns 0xFF if the queue is empty, otherwise it takes the first element into the queue, then it updates
 * the number of items and the first element of the queue (look at modulus operator).
 */
uint8_t getItemFromQueue(circularQueue_t *theQueue, msgQueueDef *theItemValue) {

    if(isEmpty(theQueue)) {
        return(0xFF);
    } else {
        theItemValue->msgID=theQueue->data[theQueue->first].msgID;
        theItemValue->msgContent=theQueue->data[theQueue->first].msgContent;
        theQueue->first=(theQueue->first+1)%MAX_ITEMS;
        theQueue->validItems--;
        return(0);
    }
}
