/*!
 * \file      max7q_cfg.h
 *
 * \brief     max7q driver configuration
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
 
#ifndef MAX7Q_CFG_H
#define MAX7Q_CFG_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include "max7q_types.h"

extern Max7q_CfgType max7q_cmd;

/* Define Hardware  */

#define	MAX7Q_HUART					        huart4

#define MAX7Q_RST_PORT              GPIOB

#define MAX7Q_RST_PIN               GPIO_PIN_3

#define MAX7Q_EXTI_PORT             GPIOB

#define MAX7Q_EXTI_PIN              GPIO_PIN_4

/* Define Debug Mode  */

#define	MAX7Q_DEBUG					1

#ifdef __cplusplus
}
#endif
#endif /* MAX7Q_CFG_H */