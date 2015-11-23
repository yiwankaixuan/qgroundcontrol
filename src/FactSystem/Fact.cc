/*=====================================================================
 
 QGroundControl Open Source Ground Control Station
 
 (c) 2009 - 2014 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 
 This file is part of the QGROUNDCONTROL project
 
 QGROUNDCONTROL is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 QGROUNDCONTROL is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with QGROUNDCONTROL. If not, see <http://www.gnu.org/licenses/>.
 
 ======================================================================*/

/// @file
///     @author Don Gagne <don@thegagnes.com>

#include "Fact.h"

#include <QtQml>

Fact::Fact(QObject* parent)
    : QObject(parent)
    , _componentId(-1)
    , _rawValue(0)
    , _type(FactMetaData::valueTypeInt32)
    , _metaData(NULL)
{    
    FactMetaData* metaData = new FactMetaData(_type, this);
    setMetaData(metaData);
}

Fact::Fact(int componentId, QString name, FactMetaData::ValueType_t type, QObject* parent)
    : QObject(parent)
    , _name(name)
    , _componentId(componentId)
    , _rawValue(0)
    , _type(type)
    , _metaData(NULL)
{
    FactMetaData* metaData = new FactMetaData(_type, this);
    setMetaData(metaData);
}

Fact::Fact(const Fact& other, QObject* parent)
    : QObject(parent)
{
    *this = other;
}

const Fact& Fact::operator=(const Fact& other)
{
    _name           = other._name;
    _componentId    = other._componentId;
    _rawValue          = other._rawValue;
    _type           = other._type;
    
    if (_metaData && other._metaData) {
        *_metaData = *other._metaData;
    } else {
        _metaData = NULL;
    }
    
    return *this;
}

void Fact::forceSetValue(const QVariant& value)
{
    if (_metaData) {
        QVariant    typedValue;
        QString     errorString;
        
        if (_metaData->convertAndValidate(value, true /* convertOnly */, typedValue, errorString)) {
            _rawValue.setValue(_metaData->cookedTranslator()(typedValue));
            emit valueChanged(typedValue);
            emit _containerValueChanged(typedValue);
        }
    } else {
        qWarning() << "Meta data pointer missing";
    }
}

void Fact::setRawValue(const QVariant& value)
{
    if (_metaData) {
        QVariant    typedValue;
        QString     errorString;
        
        if (_metaData->convertAndValidate(value, true /* convertOnly */, typedValue, errorString)) {
            if (typedValue != _rawValue) {
                _rawValue.setValue(typedValue);
                emit valueChanged(this->value());
                emit _containerValueChanged(this->value());
            }
        }
    } else {
        qWarning() << "Meta data pointer missing";
    }
}

void Fact::setValue(const QVariant& value)
{
    if (_metaData) {
        setRawValue(_metaData->cookedTranslator()(value));
    } else {
        qWarning() << "Meta data pointer missing";
    }
}

void Fact::setEnumStringValue(const QString& value)
{
    if (_metaData) {
        int index = _metaData->enumStrings().indexOf(value);
        if (index != -1) {
            setValue(_metaData->enumValues()[index]);
        }
    } else {
        qWarning() << "Meta data pointer missing";
    }
}

void Fact::setEnumIndex(int index)
{
    if (_metaData) {
        setValue(_metaData->enumValues()[index]);
    } else {
        qWarning() << "Meta data pointer missing";
    }
}

void Fact::_containerSetValue(const QVariant& value)
{
    _rawValue = value;
    emit valueChanged(_rawValue);
    emit vehicleUpdated(_rawValue);
}

QString Fact::name(void) const
{
    return _name;
}

int Fact::componentId(void) const
{
    return _componentId;
}

QVariant Fact::value(void) const
{
    if (_metaData) {
        return _metaData->rawTranslator()(_rawValue);
    } else {
        qWarning() << "Meta data pointer missing";
        return _rawValue;
    }
}

QString Fact::enumStringValue(void) const
{
    if (_metaData) {
        int enumIndex = this->enumIndex();
        if (enumIndex > 0 && enumIndex < _metaData->enumStrings().count()) {
            return _metaData->enumStrings()[enumIndex];
        }
    } else {
        qWarning() << "Meta data pointer missing";
    }

    return QString();
}

int Fact::enumIndex(void) const
{
    if (_metaData) {
        int index = 0;
        foreach (QVariant enumValue, _metaData->enumValues()) {
            if (enumValue == value()) {
                return index;
            }
            index ++;
        }
    } else {
        qWarning() << "Meta data pointer missing";
    }

    return -1;
}

QStringList Fact::enumStrings(void) const
{
    if (_metaData) {
        return _metaData->enumStrings();
    } else {
        qWarning() << "Meta data pointer missing";
        return QStringList();
    }
}

QVariantList Fact::enumValues(void) const
{
    if (_metaData) {
        return _metaData->enumValues();
    } else {
        qWarning() << "Meta data pointer missing";
        return QVariantList();
    }
}

QString Fact::_variantToString(const QVariant& variant) const
{
    QString valueString;

    switch (type()) {
        case FactMetaData::valueTypeFloat:
            valueString = QString("%1").arg(variant.toFloat(), 0, 'f', decimalPlaces());
            break;
        case FactMetaData::valueTypeDouble:
            valueString = QString("%1").arg(variant.toDouble(), 0, 'f', decimalPlaces());
            break;
        default:
            valueString = variant.toString();
            break;
    }

    return valueString;
}

QString Fact::valueString(void) const
{
    return _variantToString(value());
}

QVariant Fact::defaultValue(void) const
{
    if (_metaData) {
        if (!_metaData->defaultValueAvailable()) {
            qDebug() << "Access to unavailable default value";
        }
        return _metaData->defaultValue();
    } else {
        qWarning() << "Meta data pointer missing";
        return QVariant(0);
    }
}

QString Fact::defaultValueString(void) const
{
    return _variantToString(defaultValue());
}

FactMetaData::ValueType_t Fact::type(void) const
{
    return _type;
}

QString Fact::shortDescription(void) const
{
    if (_metaData) {
        return _metaData->shortDescription();
    } else {
        qWarning() << "Meta data pointer missing";
        return QString();
    }
}

QString Fact::longDescription(void) const
{
    if (_metaData) {
        return _metaData->longDescription();
    } else {
        qWarning() << "Meta data pointer missing";
        return QString();
    }
}

QString Fact::units(void) const
{
    if (_metaData) {
        return _metaData->units();
    } else {
        qWarning() << "Meta data pointer missing";
        return QString();
    }
}

QVariant Fact::min(void) const
{
    if (_metaData) {
        return _metaData->min();
    } else {
        qWarning() << "Meta data pointer missing";
        return QVariant(0);
    }
}

QString Fact::minString(void) const
{
    return _variantToString(min());
}

QVariant Fact::max(void) const
{
    if (_metaData) {
        return _metaData->max();
    } else {
        qWarning() << "Meta data pointer missing";
        return QVariant(0);
    }
}

QString Fact::maxString(void) const
{
    return _variantToString(max());
}

bool Fact::minIsDefaultForType(void) const
{
    if (_metaData) {
        return _metaData->minIsDefaultForType();
    } else {
        qWarning() << "Meta data pointer missing";
        return false;
    }
}

bool Fact::maxIsDefaultForType(void) const
{
    if (_metaData) {
        return _metaData->maxIsDefaultForType();
    } else {
        qWarning() << "Meta data pointer missing";
        return false;
    }
}

int Fact::decimalPlaces(void) const
{
    if (_metaData) {
        return _metaData->decimalPlaces();
    } else {
        qWarning() << "Meta data pointer missing";
        return FactMetaData::defaultDecimalPlaces;
    }
}

QString Fact::group(void) const
{
    if (_metaData) {
        return _metaData->group();
    } else {
        qWarning() << "Meta data pointer missing";
        return QString();
    }
}

void Fact::setMetaData(FactMetaData* metaData)
{
    _metaData = metaData;
    emit valueChanged(value());
}

bool Fact::valueEqualsDefault(void) const
{
    if (_metaData) {
        if (_metaData->defaultValueAvailable()) {
            return _metaData->defaultValue() == value();
        } else {
            return false;
        }
    } else {
        qWarning() << "Meta data pointer missing";
        return false;
    }
}

bool Fact::defaultValueAvailable(void) const
{
    if (_metaData) {
        return _metaData->defaultValueAvailable();
    } else {
        qWarning() << "Meta data pointer missing";
        return false;
    }
}

QString Fact::validate(const QString& value, bool convertOnly)
{
    if (_metaData) {
        
        QVariant    typedValue;
        QString     errorString;
        
        _metaData->convertAndValidate(value, convertOnly, typedValue, errorString);
        
        return errorString;
    } else {
        qWarning() << "Meta data pointer missing";
        return QString("Internal error: Meta data pointer missing");
    }
}
