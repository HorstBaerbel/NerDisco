#pragma once

#include "NodeBase.h"


class NodeRanged : public NodeBase
{
	Q_OBJECT

public:
	typedef std::shared_ptr<NodeRanged> SPtr;

	NodeRanged(const QString & name, bool value, QObject * parent = NULL);
	NodeRanged(const QString & name, int value, int minRange, int maxRange, QObject * parent = NULL);
	NodeRanged(const QString & name, float value, float minRange, float maxRange, QObject * parent = NULL);
	NodeRanged(const QString & name, double value, double minRange, double maxRange, QObject * parent = NULL);

	static QString staticTypeName();
	virtual QString typeName() const;

	virtual void toXML(QDomElement & parent) const;
	virtual void fromXML(QDomElement & parent);

	virtual void connect(NodeBase::SPtr other);
	virtual void connectNormalized(NodeRanged::SPtr other);

	NodeRanged & operator=(bool value);
	NodeRanged & operator=(int value);
	NodeRanged & operator=(float value);
	NodeRanged & operator=(double value);

	operator bool() const;
	operator int() const;
	operator float() const;
	operator double() const;

	friend bool operator==(const NodeRanged & a, const NodeRanged & b);
	friend bool operator!=(const NodeRanged & a, const NodeRanged & b);

	double value() const;
	double normalizedValue() const;
	double minRange() const;
	double maxRange() const;

public slots:
	void setOn();
	void setOff();
	void setValue(bool value);
	void setValue(int value);
	void setValue(float value);
	void setValue(double value);
	void setNormalizedValue(double value);
	void setRange(int minRange, int maxRange);
	void setRange(float minRange, float maxRange);
	void setRange(double minRange, double maxRange);

signals:
	void valueChanged(bool value);
	void valueChanged(int value);
	void valueChanged(float value);
	void valueChanged(double value);
	void normalizedValueChanged(double value);

	void rangeChanged(int minRange, int maxRange);
	void rangeChanged(float minRange, float maxRange);
	void rangeChanged(double minRange, double maxRange);

protected:
	double m_value;
	double m_minRange;
	double m_maxRange;
};
