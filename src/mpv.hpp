/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */

// C++ Wrapper for libmpv

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <mpv/client.h>
#include <mpv/render.h>
#include <mpv/render_gl.h>

namespace Mpv
{
    // Wrapper for mpv_node
    class Node : mpv_node
    {
    public:
        // Iterator
        using iterator = Node *;

        // Const iterator
        using const_iterator = const Node *;

        // Constructor
        Node() noexcept : mpv_node()
        {
            format = MPV_FORMAT_NONE;
        }

        // Move constructor
        Node(Node &&other) noexcept : mpv_node()
        {
            format       = other.format;
            u            = other.u;
            other.format = MPV_FORMAT_NONE;
        }

        // Bool constructor
        Node(bool v) noexcept : mpv_node()
        {
            format = MPV_FORMAT_FLAG;
            u.flag = v;
        }

        // Int64 constructor
        Node(int64_t v) noexcept : mpv_node()
        {
            format  = MPV_FORMAT_INT64;
            u.int64 = v;
        }

        // Double constructor
        Node(double v) noexcept : mpv_node()
        {
            format    = MPV_FORMAT_DOUBLE;
            u.double_ = v;
        }

        // String constructor
        Node(const char *v) noexcept : mpv_node()
        {
            format   = MPV_FORMAT_STRING;
            u.string = const_cast<char *>(v);
        }

        // Get type
        [[nodiscard]] mpv_format type() const noexcept
        {
            return format;
        }

        // Convert to bool
        operator bool() const noexcept
        {
            assert(format == MPV_FORMAT_FLAG);
            return u.flag;
        }

        // Convert to int64_t
        operator int64_t() const noexcept
        {
            assert(format == MPV_FORMAT_INT64);
            return u.int64;
        }

        // Convert to double
        operator double() const noexcept
        {
            assert(format == MPV_FORMAT_DOUBLE);
            return u.double_;
        }

        // Convert to string
        operator const char *() const noexcept
        {
            assert(format == MPV_FORMAT_STRING);
            return u.string;
        }

        // Equal to string
        bool operator==(const char *str) const noexcept
        {
            assert(format == MPV_FORMAT_STRING);
            return std::strcmp(u.string, str) == 0;
        }

        // Get array size
        [[nodiscard]] int size() const noexcept
        {
            assert(format == MPV_FORMAT_NODE_ARRAY);
            return u.list->num;
        }

        // Get array member
        const Node &operator[](int i) const noexcept
        {
            assert(format == MPV_FORMAT_NODE_ARRAY);
            return static_cast<Node *>(u.list->values)[i];
        }

        // Get array iterator
        [[nodiscard]] iterator begin() const noexcept
        {
            assert(format == MPV_FORMAT_NODE_ARRAY);
            return static_cast<Node *>(&u.list->values[0]);
        }

        // Get array iterator
        [[nodiscard]] iterator end() const noexcept
        {
            assert(format == MPV_FORMAT_NODE_ARRAY);
            return static_cast<Node *>(&u.list->values[u.list->num]);
        }

        // Get array iterator
        [[nodiscard]] const_iterator cbegin() const noexcept
        {
            assert(format == MPV_FORMAT_NODE_ARRAY);
            return static_cast<Node *>(&u.list->values[0]);
        }

        // Get array iterator
        [[nodiscard]] const_iterator cend() const noexcept
        {
            assert(format == MPV_FORMAT_NODE_ARRAY);
            return static_cast<Node *>(&u.list->values[u.list->num]);
        }

        // Get map member
        const Node &operator[](const char *key) const
        {
            assert(format == MPV_FORMAT_NODE_MAP);
            for (int i = 0; i < u.list->num; i++)
            {
                if (std::strcmp(key, u.list->keys[i]) == 0)
                {
                    return static_cast<Node *>(u.list->values)[i];
                }
            }
            throw std::runtime_error("Mpv::NodeMap::operator[]: key does not exist!");
        }
    };

    class Handle
    {
    private:
        mpv_handle         *m_handle = nullptr;
        mpv_render_context *m_rctx   = nullptr;

    public:
        // Initialize the object
        Handle() noexcept : m_handle(mpv_create()) {}

        // Deinitialize
        ~Handle() noexcept
        {
            if (m_rctx)
            {
                mpv_render_context_set_update_callback(m_rctx, nullptr, nullptr);
            }
            mpv_set_wakeup_callback(m_handle, nullptr, nullptr);

            if (m_rctx)
            {
                mpv_render_context_free(m_rctx);
            }
            mpv_terminate_destroy(m_handle);
        }

        // Initialize mpv after options are set
        int initialize() const noexcept
        {
            return mpv_initialize(m_handle);
        }

        // Set option
        int set_option(const char *name, const Node &data) const noexcept
        {
            return mpv_set_option(m_handle, name, MPV_FORMAT_NODE, const_cast<Node *>(&data));
        }

        // Async mpv command
        int command_async(const char **args, uint64_t reply_userdata = 0) const noexcept
        {
            return mpv_command_async(m_handle, reply_userdata, args);
        }

        // Set mpv property
        int set_property_async(const char *name, const Node &data, uint64_t reply_userdata = 0) const noexcept
        {
            return mpv_set_property_async(m_handle, reply_userdata, name, MPV_FORMAT_NODE, const_cast<Node *>(&data));
        }

        // Get mpv property
        Node get_property(const char *name) const noexcept
        {
            Node tmp;
            mpv_get_property(m_handle, name, MPV_FORMAT_NODE, &tmp);
            return tmp;
        }

        // Get mpv property in string.
        std::string get_property_string(const char *name) const noexcept
        {
            char       *s      = mpv_get_property_string(m_handle, name);
            std::string result = s ? s : std::string();
            if (s)
            {
                mpv_free(s);
            }
            return result;
        }

        // Observe mpv property.
        int observe_property(const char *name, uint64_t reply_userdata = 0) const noexcept
        {
            return mpv_observe_property(m_handle, reply_userdata, name, MPV_FORMAT_NODE);
        }

        // Wait mpv event
        [[nodiscard]] const mpv_event *wait_event(double timeout = 0) const noexcept
        {
            return mpv_wait_event(m_handle, timeout);
        }

        // Set wake up callback
        void set_wakeup_callback(void (*callback)(void *userdata), void *userdata) const noexcept
        {
            mpv_set_wakeup_callback(m_handle, callback, userdata);
        }

        // Request log messages
        int request_log_messages(const char *min_level) const noexcept
        {
            return mpv_request_log_messages(m_handle, min_level);
        }

        // Check renderer initialized
        [[nodiscard]] bool renderer_initialized() const noexcept
        {
            return m_rctx != nullptr;
        }

        // Init renderer
        int renderer_initialize(mpv_render_param *params) noexcept
        {
            return mpv_render_context_create(&m_rctx, m_handle, params);
        }

        // Set renderer callback
        void set_render_callback(mpv_render_update_fn callback, void *userdata) const noexcept
        {
            mpv_render_context_set_update_callback(m_rctx, callback, userdata);
        }

        // Render
        void render(mpv_render_param *params) const noexcept
        {
            mpv_render_context_render(m_rctx, params);
        }
    };
} // namespace Mpv