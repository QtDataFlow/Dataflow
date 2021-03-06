
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

#include "graph.h"
#include "node_time.h"

#include <dataflow/prelude/core/engine_options.h>

#include <memory>
#include <unordered_map>
#include <vector>

namespace dataflow
{
namespace internal
{
class pumpa final
{
public:
  pumpa(const memory_allocator<char>& allocator, engine_options options);

  std::size_t changed_nodes_count() const;

  std::size_t updated_nodes_count() const;

  void schedule_for_next_update(topological_position position);

  void set_metadata(const node* p_node,
                    std::shared_ptr<const metadata> p_metadata);
  const std::shared_ptr<const metadata>& get_metadata(const node* p_node);

  void pump(dependency_graph& graph,
            topological_list& order,
            vertex_descriptor time_node_v);

  bool is_pumping() const;

private:
  void pump_(dependency_graph& graph,
             topological_list& order,
             vertex_descriptor time_node_v);

private:
  const engine_options options_;
  bool pumping_started_;
  std::vector<const node*, memory_allocator<const node*>> args_buffer_;
  std::vector<topological_position, memory_allocator<topological_position>>
    next_update_;

  // TODO: move to not existing yet `network` class together with graph and
  //       topological order?
  std::unordered_map<
    const node*,
    std::shared_ptr<const metadata>,
    std::hash<const node*>,
    std::equal_to<const node*>,
    memory_allocator<
      std::pair<const node* const, std::shared_ptr<const metadata>>>>
    metadata_;

  const std::shared_ptr<const metadata> p_no_metadata_;

  std::size_t changed_nodes_count_;
  std::size_t updated_nodes_count_;
};
} // internal
} // dataflow
