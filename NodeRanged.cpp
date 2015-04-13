#include "NodeRanged.h"

#include <algorithm>


NodeRanged::NodeRanged(const QString & name, bool value, QObject * parent)
	: NodeBase(name, parent)
	, m_value(value)
	, m_minRange(0.0)
	, m_maxRange(1.0)
{
}

NodeRanged::NodeRanged(const QString & name, int value, int minRange, int maxRange, QObject * parent)
	: NodeBase(name, parent)
	, m_value(value)
	, m_minRange(minRange)
	, m_maxRange(maxRange)
{
}

NodeRanged::NodeRanged(const QString & name, float value, float minRange, float maxRange, QObject * parent)
	: NodeBase(name, parent)
	, m_value(value)
	, m_minRange(minRange)
	, m_maxRange(maxRange)
{
}

NodeRanged::NodeRanged(const QString & name, double value, double minRange, double maxRange, QObject * parent)
	: NodeBase(name, parent)
	, m_value(value)
	, m_minRange(minRange)
	, m_maxRange(maxRange)
{
}

QString NodeRanged::staticTypeName()
{
	return "NodeRanged";
}

QString NodeRanged::typeName() const
{
	return staticTypeName();
}

void NodeRanged::toXML(QDomElement & parent) const
{
	//check if element already exists
	QDomElement child = findChildElement(parent);
	//if child does not exist, create it
	if (child.isNull())
	{
		child = parent.ownerDocument().createElement(staticTypeName());
		parent.appendChild(child);
		child.setAttribute("name", m_name);
	}
	child.setAttribute("value", m_value);
	child.setAttribute("min", m_minRange);
	child.setAttribute("max", m_maxRange);
}

void NodeRanged::fromXML(QDomElement & parent)
{
	//check if element already exists
	QDomElement child = findChildElement(parent);
	//if child is not found, throw an error
	if (child.isNull())
	{
		throw std::runtime_error("Child not found in parent element!");
	}
	//child is there, read content
	m_minRange = child.attribute("min", "0.0").toDouble();
	m_maxRange = child.attribute("max", "0.0").toDouble();
	setValue(child.attribute("value", "0.0").toDouble());
}

NodeRanged & NodeRanged::operator=(bool value)
{
	setValue(value);
	return *this;
}

NodeRanged & NodeRanged::operator=(int value)
{
	setValue(value);
	return *this;
}

NodeRanged & NodeRanged::operator=(float value)
{
	setValue(value);
	return *this;
}

NodeRanged & NodeRanged::operator=(double value)
{
	setValue(value);
	return *this;
}

NodeRanged::operator bool() const
{
	return m_value > 0.0;
}

NodeRanged::operator int() const
{
	return (int)m_value;
}

NodeRanged::operator float() const
{
	return (float)m_value;
}

NodeRanged::operator double() const
{
	return m_value;
}

double NodeRanged::value() const
{
	return m_value;
}

double NodeRanged::normalizedValue() const
{
	return (m_value - m_minRange) / (m_maxRange - m_minRange);
}

double NodeRanged::minRange() const
{
	return m_minRange;
}
double NodeRanged::maxRange() const
{
	return m_maxRange;
}

void NodeRanged::setOn()
{
	setValue(m_minRange);
}

void NodeRanged::setOff()
{
	setValue(m_maxRange);
}

void NodeRanged::setValue(bool value)
{
	setValue((double)value);
}

void NodeRanged::setValue(int value)
{
	setValue((double)value);
}

void NodeRanged::setValue(float value)
{
	setValue((double)value);
}

void NodeRanged::setValue(double value)
{
	//clamp value to range
	value = std::max(m_minRange, std::min(m_maxRange, value));
	if (m_value != value)
	{
		m_value = value;
		emit valueChanged((bool)m_value);
		emit valueChanged((int)m_value);
		emit valueChanged((float)m_value);
		emit valueChanged(m_value);
		emit normalizedValueChanged(normalizedValue());
		emit changed(this);
	}
}

void NodeRanged::setNormalizedValue(double value)
{
	setValue(value * (m_maxRange - m_minRange) + m_minRange);
}

void NodeRanged::setRange(int minRange, int maxRange)
{
	setRange((double)minRange, (double)maxRange);
}

void NodeRanged::setRange(float minRange, float maxRange)
{
	setRange((double)minRange, (double)maxRange);
}

void NodeRanged::setRange(double minRange, double maxRange)
{
	//swap ranges if necessary
	if (minRange > maxRange)
	{
		std::swap(minRange, maxRange);
	}
	if (m_minRange != minRange || m_maxRange != maxRange)
	{
		m_minRange = minRange;
		m_maxRange = maxRange;
		emit rangeChanged((int)m_minRange, (int)m_maxRange);
		emit rangeChanged((float)m_minRange, (float)m_maxRange);
		emit rangeChanged(m_minRange, m_maxRange);
		emit normalizedValueChanged(normalizedValue());
		emit changed(this);
	}
}

//-------------------------------------------------------------------------------------------------

bool operator==(const NodeRanged & a, const NodeRanged & b)
{
	return (a.m_value == b.m_value && a.m_minRange == b.m_minRange && a.m_maxRange == b.m_maxRange);
}

bool operator!=(const NodeRanged & a, const NodeRanged & b)
{
	return !(a == b);
}

//-------------------------------------------------------------------------------------------------

void connect(NodeRanged::SPtr a, NodeRanged::SPtr b)
{
	QObject::connect(a.get(), SIGNAL(valueChanged(double)), b.get(), SLOT(setValue(double)));
	QObject::connect(b.get(), SIGNAL(valueChanged(double)), a.get(), SLOT(setValue(double)));
	QObject::connect(a.get(), SIGNAL(rangeChanged(double, double)), b.get(), SLOT(setRange(double, double)));
	QObject::connect(b.get(), SIGNAL(rangeChanged(double, double)), a.get(), SLOT(setRange(double, double)));
}
