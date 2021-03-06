
//  Copyright (c) 2014 - 2021 Maksym V. Bilinets.
//
//  This file is part of Dataflow++.
//
//  Dataflow++ is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Dataflow++ is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with Dataflow++. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#ifndef DATAFLOW___QT_H
#define DATAFLOW___QT_H

#include "dataflow-qt_export.h"

#include <dataflow/list.h>
#include <dataflow/prelude.h>
#include <dataflow/qt/conversion.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QObject>
#include <QtQml/QQmlEngine>

#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

namespace dataflow
{

/// \defgroup qt
/// \{

class DATAFLOW_QT_EXPORT EngineQml : public Engine
{
public:
  explicit EngineQml(const QCoreApplication& app);
  ~EngineQml();

  static EngineQml& instance();

  QQmlEngine& GetQmlEngine();

private:
  virtual ref<bool> timeout_(const ref<integer>& interval_msec,
                             dtime t0) override;

private:
  const QCoreApplication& app_;
  QQmlEngine qml_engine_;
};

namespace qt
{
template <typename T>
std::pair<std::string, var<T>> QmlPropertyRW(const std::string& name,
                                             var<T>& x);

std::pair<std::string, sig> QmlFunction(const std::string& name, const sig& x);

template <typename T,
          typename...,
          typename =
            typename std::enable_if<is_convertible_to_qml_type<T>::value>::type>
std::pair<std::string, ref<T>> QmlProperty(const std::string& name,
                                           const ref<T>& x);

DATAFLOW_QT_EXPORT std::pair<std::string, ref<qobject>>
QmlProperty(const std::string& name, const ref<listC<qvariant>>& xs);

template <typename T>
std::pair<std::string, ref<qobject>> QmlProperty(const std::string& name,
                                                 const ref<listC<T>>& xs);

template <typename... Refs>
ref<qobject> QmlContext(const std::pair<std::string, Refs>&... props);

DATAFLOW_QT_EXPORT ref<qobject> QmlComponent(const arg<std::string>& qml_url,
                                             const arg<qobject>& context);
}

/// \}
} // dataflow

#include "qt.inl"

#endif // DATAFLOW___QT_H
