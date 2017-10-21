
//  Copyright (c) 2014 - 2017 Maksym V. Bilinets.
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

#include <dataflow++_export.h>

#include "node_t.h"
#include "nodes_factory.h"
#include "ref.h"

namespace dataflow
{
namespace internal
{
DATAFLOW___EXPORT const node* update_node_state_prev(node_id id,
                                                     bool initialized);

template <typename T> class node_state_prev final : public node_t<T>
{
  friend class nodes_factory;

public:
  static ref create()
  {
    return nodes_factory::create<node_state_prev<T>>(nullptr, 0, false);
  }

private:
  explicit node_state_prev()
  : node_t<T>()
  {
  }

  virtual bool update_(node_id id,
                       bool initialized,
                       const node** p_args,
                       std::size_t args_count) override
  {
    if (auto p_state = update_node_state_prev(id, initialized))
      return this->set_value_(extract_node_value<T>(p_state));

    return false;
  }

  virtual std::string label_() const override
  {
    return "state-prev";
  }

  virtual std::pair<std::size_t, std::size_t> mem_info_() const override final
  {
    return std::make_pair(sizeof(*this), alignof(decltype(*this)));
  }
};

} // internal
} // dataflow