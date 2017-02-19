
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

#include "engine.h"
#include "config.h"
#include "node_main_activator.h"
#include "utility.h"

#include <dst/allocator/utility.h>

#include <cstdint> // std::intptr_t
#include <stack>

namespace dataflow
{
namespace internal
{
engine* engine::gp_engine_ = nullptr;

void engine::start()
{
  CHECK_PRECONDITION(gp_engine_ == nullptr);

  gp_engine_ = new engine();

  const auto p_node = static_cast<node_main_activator*>(
    dst::memory::allocate_aligned(gp_engine_->get_allocator(),
                                  sizeof(node_main_activator),
                                  alignof(node_main_activator)));

  new (std::addressof(*p_node)) node_main_activator();

  const auto v = add_vertex(vertex(p_node), gp_engine_->graph_);

  gp_engine_->graph_[v].hidden = true;

  gp_engine_->graph_[v].position =
    gp_engine_->order_.insert(gp_engine_->order_.end(), v);
}

void engine::stop()
{
  CHECK_PRECONDITION(gp_engine_ != nullptr);

  delete gp_engine_;

  gp_engine_ = nullptr;
}

vertex_descriptor
engine::add_node(node* p_node, const node_id* p_args, std::size_t args_count)
{
  CHECK_PRECONDITION(p_node != nullptr);

  const auto v = add_vertex(vertex(p_node), graph_);

  for (std::size_t i = 0; i < args_count; ++i)
  {
    const auto w = converter::convert(p_args[i]);
    add_edge(v, w, graph_);
    graph_[w].add_ref();
  }

  CHECK_POSTCONDITION(!is_active_node(v));
  CHECK_POSTCONDITION(out_degree(v, graph_) == args_count);

  return v;
}

vertex_descriptor engine::add_active_node(node* p_node, vertex_descriptor w)
{
  CHECK_PRECONDITION(p_node != nullptr);

  const auto v = add_vertex(vertex(p_node), graph_);

  edge_descriptor data_e = edge_descriptor();
  edge_descriptor logical_e = edge_descriptor();
  bool data_edge_added = false;
  bool logical_edge_added = false;

  std::tie(data_e, data_edge_added) = add_edge(v, w, graph_);

  CHECK_CONDITION(data_edge_added);

  graph_[w].add_ref();

  graph_[v].eager = true;
  graph_[v].position = order_.insert(order_.end(), v);

  std::tie(logical_e, logical_edge_added) = add_edge(v, order_.front(), graph_);

  CHECK_CONDITION(logical_edge_added);

  enable_edge(data_e);

  pump(v);

  CHECK_POSTCONDITION(is_active_node(v));

  CHECK_POSTCONDITION(out_degree(v, graph_) == 2);

  CHECK_POSTCONDITION(first_out_edge_(v) == data_e);
  CHECK_POSTCONDITION(second_out_edge_(v) == logical_e);

  CHECK_POSTCONDITION(is_active_data_dependency(data_e));
  CHECK_POSTCONDITION(is_logical_dependency(logical_e));

  return v;
}

vertex_descriptor engine::add_conditional_node(node* p_node,
                                               const node_id* p_args,
                                               std::size_t args_count)
{
  const auto v = add_node(p_node, p_args, args_count);

  graph_[v].conditional = true;

  CHECK_POSTCONDITION(is_conditional_node(v));

  return v;
}

vertex_descriptor engine::add_persistent_node(node* p_node)
{
  const auto v = add_vertex(vertex(p_node), graph_);

  graph_[v].constant = true;

  graph_[v].position = order_.begin();

  CHECK_POSTCONDITION(is_persistent_node(v));

  return v;
}

void engine::pump(vertex_descriptor v)
{
  CHECK_PRECONDITION(!pumping_started_);

  pumping_started_ = true;

  if (graph_[v].position != topological_position())
    order_.mark(graph_[v].position);

  try
  {
    pump_();
  }
  catch (...)
  {
    pumping_started_ = false;
    args_buffer_.clear();
    throw;
  }

  pumping_started_ = false;
}

// MARKER
void engine::enable_edge_(edge_descriptor e)
{
  assert(!is_active_data_dependency(e));

  const auto u = source(e, graph_);
  const auto v = target(e, graph_);
  graph_[v].consumers.push_front(u);
  graph_[e] = graph_[v].consumers.cbegin();

  assert(is_active_data_dependency(e));
}

void engine::enable_edge(edge_descriptor e)
{
  auto& g = graph_;

  assert(is_active_node(source(e, g)));
  assert(!is_active_data_dependency(e));

  struct vertex_data
  {
    edge_descriptor e;
    bool enabled;
    bool rebased;
    bool repositioned;
  };

  std::stack<vertex_data> stack;

  enable_edge_(e);

  stack.push({e, true, false, false});

  while (!stack.empty())
  {
    const auto vd = stack.top();
    stack.pop();

    const auto u = source(vd.e, g);
    const auto v = target(vd.e, g);

    assert(u != vertex_descriptor());
    assert(v != vertex_descriptor());

    if (vd.enabled)
    {
      assert(!vd.rebased);
      assert(!vd.repositioned);

      if (requires_activation(v))
      {
        assert(requires_activation(v));

        activate_vertex_(
          v, order_.insert(g[u].position, v), implied_activator_(u, v));

        order_.mark(g[v].position);

        if (g[v].conditional)
        {
          assert(out_degree(v, g) >= 2);

          const auto e = *out_edges(v, g).first;
          enable_edge_(e);
          stack.push({e, true, false, false});
        }
        else
        {
          assert(out_degree(v, g) >= 1);

          for (auto es = out_edges(v, g); es.first != es.second - 1; ++es.first)
          {
            const auto e = *es.first;
            enable_edge_(e);
            stack.push({e, true, false, false});
          }
        }
      }
      else
      {
        assert(!requires_activation(v));

        if (is_active_node(v))
        {
          const auto b = implied_activator_(u, v);

          assert(is_active_node(b));

          bool rebased = order_.order(g[b].position, g[activator_(v)].position);

          if (rebased)
          {
            reset_activator_(v, b);
          }

          bool repositioned = order_.order(g[u].position, g[v].position);

          if (repositioned)
          {
            bool marked = order_.marked(g[v].position);

            order_.erase(g[v].position);
            g[v].position = order_.insert(g[u].position, v);

            if (marked)
              order_.mark(g[v].position);
          }

          if (repositioned || rebased)
          {
            assert(out_degree(v, g) >= 1);

            for (auto es = out_edges(v, g); es.first != es.second - 1;
                 ++es.first)
            {
              if (is_active_data_dependency(*es.first))
                stack.push({*es.first, false, rebased, repositioned});
            }
          }
        }
      }
    }
    else
    {
      assert(!vd.enabled);
      assert(vd.rebased || vd.repositioned);

      if (is_active_node(v))
      {
        const auto b = implied_activator_(u, v);

        assert(is_active_node(b));

        bool rebased =
          vd.rebased && order_.order(g[b].position, g[activator_(v)].position);

        if (rebased)
        {
          reset_activator_(v, activator_(u));
        }

        bool repositioned =
          vd.repositioned && order_.order(g[u].position, g[v].position);

        if (repositioned)
        {
          bool marked = order_.marked(g[v].position);

          order_.erase(g[v].position);
          g[v].position = order_.insert(g[u].position, v);

          if (marked)
            order_.mark(g[v].position);
        }

        if (repositioned || rebased)
        {
          assert(out_degree(v, g) >= 1);

          for (auto es = out_edges(v, g); es.first != es.second - 1; ++es.first)
          {
            if (is_active_data_dependency(*es.first))
              stack.push({*es.first, false, rebased, repositioned});
          }
        }
      }
    }
  }

  return;
}

void engine::disable_edge_(edge_descriptor e)
{
  assert(is_active_data_dependency(e));

  graph_[target(e, graph_)].consumers.erase(graph_[e]);

  graph_[e] = active_edge_ticket();

  assert(!is_active_data_dependency(e));
}

void engine::disable_edge(edge_descriptor e)
{
  const auto v = target(e, graph_);

  disable_edge_(e);

  std::stack<vertex_descriptor> stack;

  stack.push(v);

  while (!stack.empty())
  {
    const auto w = stack.top();
    stack.pop();

    if (graph_[w].constant || graph_[w].position == topological_position())
      continue;

    if (graph_[w].consumers.size() == 0)
    {
      deactivate_vertex_(w);

      for (auto es = out_edges(w, graph_); es.first != es.second; ++es.first)
      {
        const auto& e = *es.first;

        if (is_active_data_dependency(e))
        {
          disable_edge_(e);

          stack.push(target(e, graph_));
        }
      }
    }
    else
    {
      const auto b = activator_(w);
      if (std::none_of(graph_[w].consumers.begin(),
                       graph_[w].consumers.end(),
                       [this, b](vertex_descriptor u)
                       {
                         return b == activator_(u);
                       }))
      {
        const auto it = std::min_element(
          graph_[w].consumers.begin(),
          graph_[w].consumers.end(),
          [this](vertex_descriptor u, vertex_descriptor v)
          {
            return order_.order(graph_[u].position, graph_[v].position);
          });

        bool marked = order_.marked(graph_[w].position);

        order_.erase(graph_[w].position);
        graph_[w].position = order_.insert(graph_[*it].position, w);

        if (marked)
          order_.mark(graph_[w].position);

        reset_activator_(w, activator_(*it));

        for (auto es = out_edges(w, graph_); es.first != es.second; ++es.first)
        {
          const auto& e = *es.first;

          if (is_active_data_dependency(e))
          {
            stack.push(target(e, graph_));
          }
        }
      }
    }
  }
}

void engine::remove_subgraph_(vertex_descriptor v)
{
  assert(graph_[v].ref_count() == 0);

  class remove_ref_dfs_visitor : public boost::default_dfs_visitor
  {
  public:
    remove_ref_dfs_visitor(dependency_graph& g)
    : graph_(g)
    {
    }

    void discover_vertex(vertex_descriptor v, const dependency_graph&)
    {
      assert(graph_[v].ref_count() == 0);

      for (auto es = out_edges(v, graph_);
           es.first !=
           (es.second - (engine::instance().is_active_node(v) ? 1 : 0));
           ++es.first)
      {
        const auto u = target(*es.first, graph_);
        if (graph_[u].release())
        {
          graph_[u].vertex_color = vertex::color::white;
        }
      }
    }

    void finish_vertex(vertex_descriptor v, const dependency_graph&)
    {
      if (graph_[v].ref_count() == 0)
      {
        engine::instance().delete_node(v);
      }
    }

  private:
    dependency_graph& graph_;
  };

  if (graph_[v].eager)
  {
    assert(out_degree(v, graph_) == 2);
    remove_edge(--out_edges(v, graph_).second, graph_);
    disable_edge(*(out_edges(v, graph_).first));
    order_.erase(graph_[v].position);
    graph_[v].position = topological_position();
  }

  graph_[v].vertex_color = vertex::color::white;

  boost::depth_first_visit(
    graph_, v, remove_ref_dfs_visitor(graph_), color_map(graph_));
}

void engine::pump_()
{
  const auto to = order_.end_marked();
  for (auto it = order_.begin_marked(); it != to; it = order_.begin_marked())
  {
    const auto v = *it;

    order_.unmark(it.base());

    const auto p_node = graph_[v].p_node;

    assert(p_node);

    assert(args_buffer_.size() == 0);

    for (auto es = out_edges(v, graph_); es.first != es.second; ++es.first)
    {
      const auto e = *es.first;

      if (is_active_data_dependency(e))
        args_buffer_.push_back(graph_[target(e, graph_)].p_node);
    }

    if (p_node->update(converter::convert(v),
                       graph_[v].initialized,
                       &args_buffer_.front(),
                       args_buffer_.size()))
    {
      for (auto v : graph_[v].consumers)
      {
        order_.mark(graph_[v].position);
      }
    }

    graph_[v].initialized = true;

    args_buffer_.clear();
  }

  assert(order_.begin_marked() == order_.end_marked());
}

engine::engine()
: allocator_()
, graph_()
, order_(allocator_)
, pumping_started_(false)
, args_buffer_(allocator_)
, ticks_()
{
}

engine::~engine() noexcept
{
  assert(num_vertices(graph_) == 1);

  delete_node(*vertices(graph_).first);
}

vertex_descriptor engine::implied_activator_(vertex_descriptor u,
                                             vertex_descriptor v) const
{
  CHECK_PRECONDITION(is_active_node(u));

  if (is_conditional_node(u))
  {
    const auto w = target(first_out_edge_(u), graph_);

    if (v != w)
      return w;
  }

  return target(last_out_edge_(u), graph_);
}

void engine::activate_vertex_(vertex_descriptor v,
                              topological_position pos,
                              vertex_descriptor w)
{
  CHECK_PRECONDITION(requires_activation(v));
  CHECK_PRECONDITION(*pos == v);

  graph_[v].position = pos;

  add_edge(v, w, graph_);

  CHECK_POSTCONDITION(!graph_[v].initialized);
  CHECK_POSTCONDITION(is_active_node(v));
}

void engine::deactivate_vertex_(vertex_descriptor v)
{
  CHECK_PRECONDITION(is_active_node(v));
  CHECK_PRECONDITION(graph_[v].initialized);
  CHECK_PRECONDITION(graph_[v].consumers.size() == 0);

  order_.erase(graph_[v].position);
  graph_[v].position = topological_position();

  remove_edge(last_out_edge_(v), graph_);

  graph_[v].initialized = false;

  CHECK_POSTCONDITION(!graph_[v].initialized);
  CHECK_POSTCONDITION(requires_activation(v));
}

void engine::reset_activator_(vertex_descriptor v, vertex_descriptor w)
{
  CHECK_PRECONDITION(is_active_node(v));
  CHECK_PRECONDITION(is_logical_dependency(last_out_edge_(v)));

  remove_edge(last_out_edge_(v), graph_);

  add_edge(v, w, graph_);
}

} // internal
} // dataflow
