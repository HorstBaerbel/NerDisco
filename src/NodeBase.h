#pragma once

#include <memory>

#include <QObject>
#include <QString>
#include <QDomElement>


class NodeBase : public QObject
{
	Q_OBJECT

public:
	typedef std::shared_ptr<NodeBase> SPtr;

	NodeBase(const QString & name, QObject * parent = NULL);

	virtual void toXML(QDomElement & element) const = 0;
	virtual void fromXML(QDomElement & element) = 0;

	virtual QString typeName() const = 0;
	QString name() const;
	void setName(const QString & name);

signals:
	void changed(NodeBase * node);

protected:
	QDomElement findChildElement(QDomElement & parent) const;

	QString m_name;
};
