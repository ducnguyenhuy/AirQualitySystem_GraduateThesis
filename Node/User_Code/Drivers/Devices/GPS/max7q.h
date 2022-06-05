/*!
 * \file      max7q.h
 *
 * \brief     max7q driver header
 *
 * \copyright @SANSLAB
 *
 * \code
 *              _____         _   _  _____ _               ____  
 *             / ____|  /\   | \ | |/ ____| |        /\   |  _ \ 
 *            | (___   /  \  |  \| | (___ | |       /  \  | |_) |
 *             \___ \ / /\ \ | . ` |\___ \| |      / /\ \ |  _ < 
 *             ____) / ____ \| |\  |____) | |____ / ____ \| |_) |
 *            |_____/_/    \_\_| \_|_____/|______/_/    \_\____/ 
 *            (C)Smart Applications and Network System Laboratory
 *
 * \endcode
 *
 * \author    Trung Ha Nguyen ( Sanslab )
 */
 
#ifndef MAX7Q_H
#define MAX7Q_H

#ifdef __cplusplus
extern "C" {
#endif

#include "./cfg/max7q_cfg.h"

void Max7q_Init(Max7q_t *max7q);

void Max7q_CallBack(Max7q_t *max7q);

void Max7q_Process(Max7q_t *max7q);
	
#ifdef __cplusplus
}
#endif
#endif /* MAX7Q_H */