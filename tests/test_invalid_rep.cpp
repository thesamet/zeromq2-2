/*
    Copyright (c) 2007-2011 iMatix Corporation
    Copyright (c) 2007-2011 Other contributors as noted in the AUTHORS file

    This file is part of 0MQ.

    0MQ is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    0MQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "testutil.hpp"
#include "../src/stdint.hpp"

void send_invalid_multipart (void *socket, zmq_msg_t *first)
{
    int rc;
    zmq_msg_t part1, part2, part3;

    zmq_msg_init_size (&part1, zmq_msg_size (first));
    memcpy (zmq_msg_data (&part1), zmq_msg_data (first), zmq_msg_size (first));

    rc = zmq_send (socket, &part1, 0);
    assert (rc == 0);

    zmq_msg_init (&part2);
    rc = zmq_send (socket, &part2, ZMQ_SNDMORE);
    assert (rc == 0);

    zmq_msg_init_size (&part3, 1);
    memcpy (zmq_msg_data (&part3), "a", 1);

    rc = zmq_send (socket, &part3, 0);
    assert (rc == 0);
}

void send_valid_multipart (void *socket, zmq_msg_t *first)
{
    int rc;
    zmq_msg_t part1, part2, part3;

    zmq_msg_init_size (&part1, zmq_msg_size (first));
    memcpy (zmq_msg_data (&part1), zmq_msg_data (first), zmq_msg_size (first));

    rc = zmq_send (socket, &part1, ZMQ_SNDMORE);
    assert (rc == 0);

    zmq_msg_init (&part2);
    rc = zmq_send (socket, &part2, ZMQ_SNDMORE);
    assert (rc == 0);

    zmq_msg_init_size (&part3, 1);
    memcpy (zmq_msg_data (&part3), "b", 1);

    rc = zmq_send (socket, &part3, 0);
    assert (rc == 0);
}

int main (int argc, char *argv [])
{
    int rc, linger = 0;

    void *ctx;
    void *xrep_socket, *req_socket;

    ctx = zmq_init (1);
    assert (ctx);

    xrep_socket = zmq_socket (ctx, ZMQ_XREP);
    assert (xrep_socket);

    req_socket = zmq_socket (ctx, ZMQ_REQ);
    assert (req_socket);

    zmq_setsockopt (xrep_socket, ZMQ_LINGER, &linger, sizeof (int));
    zmq_setsockopt (req_socket, ZMQ_LINGER, &linger, sizeof (int));

    zmq_bind (xrep_socket, "inproc://hi");
    zmq_connect (req_socket, "inproc://hi");

    // Initial request
    zmq_msg_t request;
    zmq_msg_init_size (&request, 1);
    memcpy (zmq_msg_data (&request), "r", 1);
    rc = zmq_send (req_socket, &request, 0);
    assert (rc == 0);

    // Receive the request
    zmq_msg_t request_pt1;
    zmq_msg_init (&request_pt1);
    rc = zmq_recv (xrep_socket, &request_pt1, 0);
    assert (rc == 0);

    zmq_msg_t request_pt2;
    zmq_msg_init (&request_pt2);
    rc = zmq_recv (xrep_socket, &request_pt2, 0);
    assert (rc == 0);

    zmq_msg_t request_pt3;
    zmq_msg_init (&request_pt3);
    rc = zmq_recv (xrep_socket, &request_pt3, 0);
    assert (rc == 0);

    // Send invalid multipart
    send_invalid_multipart (xrep_socket, &request_pt1);

    // Now send a valid multipart
    send_valid_multipart (xrep_socket, &request_pt1);

    zmq_msg_t valid;
    zmq_msg_init (&valid);
    rc = zmq_recv (req_socket, &valid, 0);
    assert (rc == 0);

    char *data = (char *) zmq_msg_data (&valid);
    assert (data [0] == 'b');

    rc = zmq_close (xrep_socket);
    assert (rc == 0);

    rc = zmq_close (req_socket);
    assert (rc == 0);

    rc = zmq_term (ctx);
    assert (rc == 0);

    return 0;
}
