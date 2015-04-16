#pragma once

#include "NodeBase.h"


class NodeQString : public NodeBase
{
	Q_OBJECT

public:
	typedef std::shared_ptr<NodeQString> SPtr;

	NodeQString(const QString & name, QString value, QObject * parent = NULL);

	static QString staticTypeName();
	virtual QString typeName() const;

	virtual void toXML(QDomElement & parent) const;
	virtual void fromXML(QDomElement & parent);

	virtual void connect(NodeBase::SPtr other);

	NodeQString & operator=(const QString & value);
	operator QString() const;

	friend bool operator==(const NodeQString & a, const NodeQString & b);
	friend bool operator!=(const NodeQString & a, const NodeQString & b);

	const QString & value() const;

public slots:
	void setValue(const QString & value);

signals:
	void valueChanged(const QString & value);

protected:
	QString m_value;
};
