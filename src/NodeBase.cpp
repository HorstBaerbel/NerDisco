#include "NodeBase.h"


NodeBase::NodeBase(const QString & name, QObject * parent)
	: QObject(parent)
	, m_name(name)
{
}

QString NodeBase::name() const
{
	return m_name;
}

void NodeBase::setName(const QString & name)
{
	m_name = name;
}

QDomElement NodeBase::findChildElement(QDomElement & parent) const
{
	if (parent.isNull())
	{
		throw std::runtime_error("Bad XML parent element!");
	}
	//check if element already exists
	QDomElement child;
	QDomNodeList children = parent.childNodes();
	for (int i = 0; i < children.size(); ++i)
	{
		QDomElement childAti = children.at(i).toElement();
		if (!childAti.isNull() && childAti.tagName() == typeName() && childAti.attribute("name") == m_name)
		{
			child = childAti;
			break;
		}
	}
	return child;
}
