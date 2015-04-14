#pragma once

#include <QVector>
#include <QImage>
#include <QObject>
#include <QMutex>


class SignalJoiner : public QObject
{
	Q_OBJECT

public:
	~SignalJoiner();

	void addObjectToJoin(QObject * object);
	void removeObjectToJoin(QObject * object);

	void start();
	void stop();

	bool isJoining() const;

public slots:
	void notify();

signals:
	void joined();

private:
	mutable QMutex m_listMutex;
	QVector<QObject*> m_objectList;
	QVector<QObject*> m_waitList;
};
