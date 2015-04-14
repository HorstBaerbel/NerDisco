#include "NodeEnum.h"


NodeEnum::NodeEnum(const QString & name, int64_t value, QObject * parent)
	: NodeBase(name, parent)
	, m_value(value)
{
}

void NodeEnum::toXML(QDomElement & parent) const
{
	//check if element already exists
	QDomElement child = findChildElement(parent);
	//if child does not exist, create it
	if (child.isNull())
	{
		child = parent.ownerDocument().createElement(typeName());
		child.setAttribute("name", m_name);
		parent.appendChild(child);
	}
	child.setAttribute("value", m_value);
}

void NodeEnum::fromXML(QDomElement & parent)
{
	//check if element already exists
	QDomElement child = findChildElement(parent);
	//if child is not found, throw an error
	if (child.isNull())
	{
		throw std::runtime_error("Child not found in parent element!");
	}
	//child is there, read content
	setValue(child.attribute("value", "").toInt());
}

void NodeEnum::addValue(int64_t value, const QString & name)
{
	if (m_entries.contains(value))
	{
		throw std::runtime_error("NodeEnum::addValue() - Enumeration value already exists!");
	}
	m_entries[value] = name;
}

NodeEnum & NodeEnum::operator=(int64_t value)
{
	setValue(value);
	return *this;
}

NodeEnum & NodeEnum::operator=(const QString & value)
{
	setValue(value);
	return *this;
}

NodeEnum::operator int64_t() const
{
	return m_value;
}

NodeEnum::operator QString() const
{
	return m_entries[m_value];
}

/*int64_t NodeEnum::value() const
{
	return m_value;
}*/

QString NodeEnum::valueName(int64_t value) const
{
	return m_entries[value];
}

void NodeEnum::setValue(int64_t value)
{
	if (m_value != value)
	{
		if (!m_entries.contains(value))
		{
			throw std::runtime_error("NodeEnum::setValue() - Invalid enumeration value!");
		}
		m_value = value;
		emitValueChanged();
		emit valueChanged(m_value);
		emit valueChanged(m_entries[m_value]);
		emit changed(this);
	}
}

void NodeEnum::setValue(const QString & value)
{
	auto iter = m_entries.cbegin();
	while (iter != m_entries.cend())
	{
		if (iter.value() == value)
		{
			setValue(iter.key());
			return;
		}
		++iter;
	}
	throw std::runtime_error("NodeEnum::setValue() - Invalid enumeration value name!");
}

//-------------------------------------------------------------------------------------------------

bool operator==(const NodeEnum & a, const NodeEnum & b)
{
	return (a.m_value == b.m_value && a.m_entries == b.m_entries);
}

bool operator!=(const NodeEnum & a, const NodeEnum & b)
{
	return !(a == b);
}

//-------------------------------------------------------------------------------------------------

void connect(NodeEnum::SPtr a, NodeEnum::SPtr b)
{
	QObject::connect(a.get(), SIGNAL(valueChanged(int64_t)), b.get(), SLOT(setValue(int64_t)));
	QObject::connect(b.get(), SIGNAL(valueChanged(int64_t)), a.get(), SLOT(setValue(int64_t)));
}
