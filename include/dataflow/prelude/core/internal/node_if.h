
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

#include "config.h"
#include "node_t.h"
#include "nodes_factory.h"
#include "ref.h"

#include <array>
#include <utility>

namespace dataflow
{
namespace internal
{
template <typename T> class node_if final : public node_t<T>
{
  friend class nodes_factory;

public:
  static ref create(const ref& basis,
                    const ref& consequence,
                    const ref& alternative,
                    bool eager)
  {
    DATAFLOW___CHECK_PRECONDITION(basis.template is_of_type<bool>());
    DATAFLOW___CHECK_PRECONDITION(consequence.template is_of_type<T>());
    DATAFLOW___CHECK_PRECONDITION(alternative.template is_of_type<T>());

    const std::array<node_id, 3> args = {
      {basis.id(), alternative.id(), consequence.id()}};

    return nodes_factory::create_conditional<node_if<T>>(
      &args[0], args.size(), eager ? node_flags::eager : node_flags::none);
  }

private:
  explicit node_if()
  : p_prev_branch_()
  {
  }

  virtual update_status update_(node_id id,
                                bool initialized,
                                const node** p_args,
                                std::size_t args_count) override
  {
    DATAFLOW___CHECK_PRECONDITION(p_args != nullptr);
    DATAFLOW___CHECK_PRECONDITION(args_count == 2);

    if (initialized && p_prev_branch_ == p_args[1])
      node::set_metadata(this, node::get_metadata(p_args[1]));

    p_prev_branch_ = p_args[1];

    return this->set_value_(extract_node_value<T>(p_args[1]));
  }

  virtual void deactivate_(node_id id) override
  {
    p_prev_branch_ = nullptr;

    node_t<T>::perform_deactivation_();
  }

  virtual std::string label_() const override
  {
    return "if";
  }

  virtual std::pair<std::size_t, std::size_t> mem_info_() const override final
  {
    return std::make_pair(sizeof(*this), alignof(decltype(*this)));
  }

private:
  const node* p_prev_branch_; // TODO: try to avoid this cache
};
} // internal
} // dataflow
