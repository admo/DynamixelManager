/*
 * baudrates.h
 *
 *  Created on: 03-08-2010
 *      Author: aoleksy
 */

#ifndef BAUDRATES_H_
#define BAUDRATES_H_

#include <QPair>
#include <QString>
#include <QVector>
#include <vector>

namespace BaudRate {
typedef QPair<QString, int> BaudRateNamePair;

const BaudRateNamePair baudRates[] = {
		BaudRateNamePair(QString("50"), 50),
		BaudRateNamePair(QString("75"), 75),
		BaudRateNamePair(QString("110"), 110),
		BaudRateNamePair(QString("134"), 134),
		BaudRateNamePair(QString("150"), 150),
		BaudRateNamePair(QString("200"), 200),
		BaudRateNamePair(QString("300"), 300),
		BaudRateNamePair(QString("600"), 600),
		BaudRateNamePair(QString("1200"), 1200),
		BaudRateNamePair(QString("1800"), 1800),
		BaudRateNamePair(QString("2400"), 2400),
		BaudRateNamePair(QString("4800"), 4800),
		BaudRateNamePair(QString("9600"), 9600),
		BaudRateNamePair(QString("19200"), 19200),
		BaudRateNamePair(QString("38400"), 38400),
		BaudRateNamePair(QString("57600"), 57600),
		BaudRateNamePair(QString("115200"), 115200),
		BaudRateNamePair(QString("230400"), 230400)
};

}
#endif /* BAUDRATES_H_ */
