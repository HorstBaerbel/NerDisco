#pragma once

#include "NodeEnum.h"
#include "ParameterT.h"


enum ScanlineDirection {
	ConstantLeftToRight, ConstantRightToLeft,
	AlternatingStartLeft, AlternatingStartRight
};

class NodeScanlineDirection : public NodeEnum
{
	Q_OBJECT

public:
	NodeScanlineDirection(const QString & name, ScanlineDirection value, QObject * parent = NULL);
	static QString staticTypeName();
	QString typeName() const;

	ScanlineDirection value() const;

public slots:
	void setValue(ScanlineDirection value);

signals:
	void valueChanged(ScanlineDirection value);

protected:
	virtual void emitValueChanged();
};

typedef ParameterT<ScanlineDirection, NodeScanlineDirection, false> ParameterScanlineDirection;
