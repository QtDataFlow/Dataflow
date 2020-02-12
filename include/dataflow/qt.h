
//  Copyright (c) 2014 - 2020 Maksym V. Bilinets.
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

#include <dataflow/prelude/core.h>

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

  static EngineQml& instance();

  QQmlEngine& GetQmlEngine();

private:
  QQmlEngine qml_engine_;
};

namespace qt
{
template <typename... Ts>
std::tuple<std::pair<std::string, std::reference_wrapper<var<Ts>>>...>
RW(const std::pair<std::string, std::reference_wrapper<var<Ts>>>&... props);

template <typename T>
std::pair<std::string, std::reference_wrapper<var<T>>>
QmlPropertyRW(const std::string& name, var<T>& x);

template <typename T>
std::pair<std::string, ref<T>> QmlProperty(const std::string& name,
                                           const ref<T>& x);

template <typename... Ts, typename... Us>
ref<std::shared_ptr<QObject>> QmlContext(
  const std::tuple<std::pair<std::string, std::reference_wrapper<var<Ts>>>...>&
    rw_props,
  const std::pair<std::string, ref<Us>>&... props);
}

/// \}
} // dataflow

#include "qt.inl"

#endif // DATAFLOW___QT_H
