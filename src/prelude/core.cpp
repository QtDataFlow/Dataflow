
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

#include <dataflow/prelude/core.h>

#include "core/internal/engine.h"

#include <dataflow/prelude/core/internal/node_signal.h>

namespace dataflow
{
Engine::Engine()
{
  internal::engine::start(this);
}

Engine::~Engine()
{
  internal::engine::stop();
}

Engine* Engine::engine_()
{
  return static_cast<Engine*>(internal::engine::data());
}

void sig::emit() const
{
  this->schedule_();
}

sig::sig(const internal::ref& r, internal::ref::ctor_guard_t)
: ref<bool>(core::ref_base<bool>(r, internal::ref::ctor_guard))
{
}

} // dataflow

dataflow::ref<std::size_t> dataflow::CurrentTime()
{
  return core::ref_base<std::size_t>(internal::nodes_factory::get_time(),
                                     internal::ref::ctor_guard);
}

dataflow::sig dataflow::Signal()
{
  return sig(internal::node_signal::create(), internal::ref::ctor_guard);
}
