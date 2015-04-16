#include "NodeQString.h"


NodeQString::NodeQString(const QString & name, QString value, QObject * parent)
	: NodeBase(name, parent)
	, m_value(value)
{
}

QString NodeQString::staticTypeName()
{
	return "NodeQString";
}

QString NodeQString::typeName() const
{
	return staticTypeName();
}

void NodeQString::toXML(QDomElement & parent) const
{
	//check if element already exists
	QDomElement child = findChildElement(parent);
	//if child does not exist, create it
	if (child.isNull())
	{
		child = parent.ownerDocument().createElement(staticTypeName());
		child.setAttribute("name", m_name);
		parent.appendChild(child);
	}
	child.setAttribute("value", m_value);
}

void NodeQString::fromXML(QDomElement & parent)
{
	//check if element already exists
	QDomElement child = findChildElement(parent);
	//if child is not found, throw an error
	if (child.isNull())
	{
		throw std::runtime_error("Child not found in parent element!");
	}
	//child is there, read content
	setValue(child.attribute("value", ""));
}

void NodeQString::connect(NodeBase::SPtr other)
{
	if (this == other.get())
	{
		throw std::runtime_error("NodeQString::connect() - Can't connect a node to itself!");
	}
	NodeQString * otherNode = qobject_cast<NodeQString*>(other.get());
	if (otherNode == nullptr)
	{
		throw std::runtime_error("NodeQString::connect() - Only nodes of type NodeQString can be connected!");
	}
	QObject::connect(this, SIGNAL(valueChanged(QString)), other.get(), SLOT(setValue(QString)));
	QObject::connect(other.get(), SIGNAL(valueChanged(QString)), this, SLOT(setValue(QString)));
}

NodeQString & NodeQString::operator=(const QString & value)
{
	setValue(value);
	return *this;
}

NodeQString::operator QString() const
{
	return m_value;
}

const QString & NodeQString::value() const
{
	return m_value;
}

void NodeQString::setValue(const QString & value)
{
	if (m_value != value)
	{
		m_value = value;
		emit valueChanged(m_value);
		emit changed(this);
	}
}

//-------------------------------------------------------------------------------------------------

bool operator==(const NodeQString & a, const NodeQString & b)
{
	return (a.m_value == b.m_value);
}

bool operator!=(const NodeQString & a, const NodeQString & b)
{
	return !(a == b);
}

//-------------------------------------------------------------------------------------------------

void connect(NodeQString::SPtr a, NodeQString::SPtr b)
{
	QObject::connect(a.get(), SIGNAL(valueChanged(const QString &)), b.get(), SLOT(setValue(const QString &)));
	QObject::connect(b.get(), SIGNAL(valueChanged(const QString &)), a.get(), SLOT(setValue(const QString &)));
}
