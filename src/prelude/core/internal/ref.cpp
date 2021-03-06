
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

#include <dataflow/prelude/core/internal/ref.h>

#include "engine.h"

namespace dataflow
{

namespace internal
{
ref::ref(const ref& other)
: id_(other.id_)
{
  engine::instance().add_ref(converter::convert(id_));
}

ref::~ref()
{
  engine::instance().release(converter::convert(id_));
}

const node* ref::get_() const
{
  return engine::instance().get_node(converter::convert(id_));
}

void ref::schedule_() const
{
  if (engine::instance().is_active_node(converter::convert(id_)))
  {
    if (engine::instance().is_pumping())
    {
      engine::instance().schedule_for_next_update(converter::convert(id_));
    }
    else
    {
      engine::instance().schedule_and_pump(converter::convert(id_));
    }
  }
}

void ref::set_metadata(std::shared_ptr<const metadata> p_metadata)
{
  if (engine::instance().is_active_node(converter::convert(id_)))
    engine::instance().set_metadata(get_(), std::move(p_metadata));
}

void ref::reset_(const ref& other)
{
  engine::instance().release(converter::convert(id_));

  id_ = other.id_;

  engine::instance().add_ref(converter::convert(id_));
}

ref::ref(node_id id)
: id_(id)
{
  engine::instance().add_ref(converter::convert(id_));
}
}
}
