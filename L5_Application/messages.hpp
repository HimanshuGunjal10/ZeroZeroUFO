/*
 * messages.hpp
 *
 *  Created on: Dec 4, 2018
 *      Author: ollie
 */

#ifndef MESSAGES_HPP_
#define MESSAGES_HPP_


#include "wireless.h"
#include "sys_config.h"


/* Message opcodes */
const uint32_t ZZUM_MOVE = 0;
const uint32_t ZZUM_CTRL = 100;

/* Speeds */
enum zzu_velocity {
    df = 0,     /* down fast */
    dm,         /* down medium, etc... */
    ds,
    no,         /* no move */
    us,
    um,
    uf
};


#endif /* MESSAGES_HPP_ */
