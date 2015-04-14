#pragma once

#include <memory>
#include <QtCore/QObject>
#include <QtXml/QDomElement>


template<typename VALUETYPE, typename NODETYPE, bool RANGED = true>
class ParameterT
{
public:
	template <bool R = RANGED>
	ParameterT(const QString & name, const typename std::enable_if<R, VALUETYPE>::type & value, const VALUETYPE & minRange, const VALUETYPE & maxRange, QObject * parent = NULL)
		: m_parameter(new NODETYPE(name, value, minRange, maxRange, parent))
	{
	}

	template <bool R = RANGED>
	ParameterT(const QString & name, const typename std::enable_if<!R, VALUETYPE>::type & value, QObject * parent = NULL)
		: m_parameter(new NODETYPE(name, value, parent))
	{
	}

	ParameterT(std::shared_ptr<NODETYPE> parameter)
		: m_parameter(parameter)
	{
		if (!m_parameter)
		{
			throw std::runtime_error("ParameterT - Invalid parameter passed!");
		}
	}

	QString name() const
	{
		return m_parameter->name();
	}

	void toXML(QDomElement & element) const
	{
		m_parameter->toXML(element);
	}

	void fromXML(QDomElement & element)
	{
		m_parameter->fromXML(element);
	}

	std::shared_ptr<NODETYPE> GetSharedParameter()
	{
		return m_parameter;
	}

	ParameterT<VALUETYPE, NODETYPE> & operator=(const ParameterT<VALUETYPE, NODETYPE> & value)
	{
		m_parameter->setValue(value);
		return *this;
	}

	ParameterT<VALUETYPE, NODETYPE, RANGED> & operator=(VALUETYPE value)
	{
		m_parameter->setValue(value);
		return *this;
	}

	operator VALUETYPE() const
	{
		return m_parameter->value();
	}

	template <bool R = RANGED>
	typename std::enable_if<R, VALUETYPE>::type minRange() const
	{
		return m_parameter->minRange();
	}

	template <bool R = RANGED>
	typename std::enable_if<R, VALUETYPE>::type maxRange() const
	{
		return m_parameter->maxRange();
	}

	float normalizedValue() const
	{
		return m_parameter->normalizedValue();
	}

	/*friend bool operator==(const ParameterT<VALUETYPE, NODETYPE, RANGED> & a, const ParameterT<VALUETYPE, NODETYPE, RANGED> & b);
	friend bool operator==(const ParameterT<VALUETYPE, NODETYPE, RANGED> & a, const VALUETYPE & b);
	friend bool operator==(const VALUETYPE & a, const ParameterT<VALUETYPE, NODETYPE, RANGED> & b);

	friend bool operator!=(const ParameterT<VALUETYPE, NODETYPE, RANGED> & a, const ParameterT<VALUETYPE, NODETYPE, RANGED> & b);
	friend bool operator!=(const ParameterT<VALUETYPE, NODETYPE, RANGED> & a, const VALUETYPE & b);
	friend bool operator!=(const VALUETYPE & a, const ParameterT<VALUETYPE, NODETYPE, RANGED> & b);*/

protected:
	std::shared_ptr<NODETYPE> m_parameter;
};

/*template<typename VALUETYPE, typename NODETYPE, bool RANGED = true>
bool operator==(const ParameterT<VALUETYPE, NODETYPE, RANGED> & a, const ParameterT<VALUETYPE, NODETYPE, RANGED> & b)
{
	return (*a.m_parameter == *b.m_parameter);
}

template<typename VALUETYPE, typename NODETYPE, bool RANGED = true>
bool operator==(const ParameterT<VALUETYPE, NODETYPE, RANGED> & a, const VALUETYPE & b)
{
	return (*a.m_parameter->value() == b);
}

template<typename VALUETYPE, typename NODETYPE, bool RANGED = true>
bool operator==(const VALUETYPE & a, const ParameterT<VALUETYPE, NODETYPE, RANGED> & b)
{
	return (a == *b.m_parameter->value());
}

template<typename VALUETYPE, typename NODETYPE, bool RANGED = true>
bool operator!=(const ParameterT<VALUETYPE, NODETYPE, RANGED> & a, const ParameterT<VALUETYPE, NODETYPE, RANGED> & b)
{
	return !(a == b);
}

template<typename VALUETYPE, typename NODETYPE, bool RANGED = true>
bool operator!=(const ParameterT<VALUETYPE, NODETYPE, RANGED> & a, const VALUETYPE & b)
{
	return !(a == b);
}

template<typename VALUETYPE, typename NODETYPE, bool RANGED = true>
bool operator!=(const VALUETYPE & a, const ParameterT<VALUETYPE, NODETYPE, RANGED> & b)
{
	return !(a == b);
}*/

template<typename VALUETYPE, typename NODETYPE, bool RANGED = true>
void connectParameter(ParameterT<VALUETYPE, NODETYPE, RANGED> & a, ParameterT<VALUETYPE, NODETYPE, RANGED> & b)
{
	if (a.GetSharedParameter() == b.GetSharedParameter())
	{
		throw std::runtime_error("connectParameter() - Connecting a parameter to itself is not allowed!");
	}
	connect(a.GetSharedParameter(), b.GetSharedParameter());
}
