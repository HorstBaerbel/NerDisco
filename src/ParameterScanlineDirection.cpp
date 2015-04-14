#include "ParameterScanlineDirection.h"


NodeScanlineDirection::NodeScanlineDirection(const QString & name, ScanlineDirection value, QObject * parent)
	: NodeEnum(name, value, parent)
{
	m_entries[ScanlineDirection::ConstantLeftToRight] = "ConstantLeftToRight";
	m_entries[ScanlineDirection::ConstantRightToLeft] = "ConstantRightToLeft";
	m_entries[ScanlineDirection::AlternatingStartLeft] = "AlternatingStartLeft";
	m_entries[ScanlineDirection::AlternatingStartRight] = "AlternatingStartRight";
}

QString NodeScanlineDirection::staticTypeName()
{
	return "NodeScanlineDirection";
}

QString NodeScanlineDirection::typeName() const
{
	return staticTypeName();
}

ScanlineDirection NodeScanlineDirection::value() const
{
	return (ScanlineDirection)m_value;
}

void NodeScanlineDirection::setValue(ScanlineDirection value)
{
	NodeEnum::setValue((int64_t)value);
}

void NodeScanlineDirection::emitValueChanged()
{
	emit valueChanged((ScanlineDirection)m_value);
}
