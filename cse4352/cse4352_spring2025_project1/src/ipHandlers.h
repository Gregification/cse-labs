/*
 * ipHandlers.h
 *
 *  Created on: Feb 16, 2025
 *      Author: turtl
 */

#ifndef SRC_IPHANDLERS_H_
#define SRC_IPHANDLERS_H_

#include "env.h"
#include "framework/socket.h"

/**
 * starts a async TCP connection
 * @param ethh : buffer to write to. to minimize memory claimed
 * @param sock : the tcp connection socket to use
 * @param handler : arbitration. if a connection is established, the resolver will be passed any packets the socket receives.
 *      the callback will also be called if the connection times out at any point.
 * @return false if unable to start hand shake
 */
bool startTCPConnection(etherHeader * ethh, socket * sock, ethHandler dataHandler);

/**
 * updates a TCP connection using the provided ethernet packet (if it is relevant)
 * @param sock : the tcp connection socket
 * @param ethh : must be a IP packet
 */
void maintainTCPConnection(socket * sock, etherHeader * ethh);

#endif /* SRC_IPHANDLERS_H_ */
