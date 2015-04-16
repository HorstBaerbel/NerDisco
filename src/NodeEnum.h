#pragma once

#include "NodeBase.h"
#include <QtCore/QMap>


class NodeEnum : public NodeBase
{
	Q_OBJECT

public:
	typedef std::shared_ptr<NodeEnum> SPtr;

	virtual void toXML(QDomElement & parent) const;
	virtual void fromXML(QDomElement & parent);

	virtual void connect(NodeBase::SPtr other);

	void addValue(int64_t value, const QString & name = "");
	NodeEnum & operator=(int64_t value);
	NodeEnum & operator=(const QString & value);
	operator int64_t() const;
	operator QString() const;

	friend bool operator==(const NodeEnum & a, const NodeEnum & b);
	friend bool operator!=(const NodeEnum & a, const NodeEnum & b);

	//int64_t value() const;
	QString valueName(int64_t value) const;

public slots:
	void setValue(int64_t value);
	void setValue(const QString & value);

signals:
	void valueChanged(int64_t value);
	void valueChanged(const QString & value);

protected:
	NodeEnum(const QString & name, int64_t value, QObject * parent = NULL);
	virtual void emitValueChanged() = 0;

	int64_t m_value;
	QMap<int64_t, QString> m_entries;
};
