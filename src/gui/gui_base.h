#pragma once

#include <set>
#include <vector>
#include <algorithm>
#include <memory>

namespace drivers {
class driver_base;
}

namespace gui {

template <class T>
class gui_base {
protected:
    explicit gui_base(std::shared_ptr<drivers::driver_base> d, T *p = nullptr, int z = 0): driver(std::move(d)), parent(p), zorder(z) {
        if (p) {
            p->add(this);
        }
    }
    virtual ~gui_base() {
        if (parent) {
            parent->remove(this);
            parent = nullptr;
        }
    }

public:
    /* enter event loop */
    virtual void event_loop() {}
    /* call this to leave event loop */
    virtual void leave_event_loop() {
        running = false;
        if (parent) {
            parent->leave_event_loop();
        }
    }

    /* draw element */
    virtual void draw() = 0;

private:
    struct child_zorder_comp {
        inline bool operator() (const gui_base *child, int z) const {
            return child->zorder < z;
        }
        inline bool operator() (int z, const gui_base *child) const {
            return z < child->zorder;
        }
    };
    inline void add(gui_base *child) {
        auto ite = children.find(child);
        if (ite != children.end()) {
            return;
        }
        children.insert(child);
        children_ordered.insert(std::upper_bound(children_ordered.begin(), children_ordered.end(), child->zorder, child_zorder_comp()), child);
    }
    inline void remove(gui_base *child) {
        children.erase(child);
        auto z = child->zorder;
        auto ite = std::lower_bound(children_ordered.begin(), children_ordered.end(), child->zorder, child_zorder_comp());
        while (ite != children_ordered.end()) {
            if ((*ite)->zorder != z) {
                break;
            }
            if (*ite == child) {
                children_ordered.erase(ite);
                break;
            }
            ++ite;
        }
    }

protected:
    /* called when entering gui mode, doing initialization here */
    virtual void init() {}
    /* called when leaving gui mode, doing deinitialization here */
    virtual void deinit() {}

protected:
    std::shared_ptr<drivers::driver_base> driver;

    T *parent = nullptr;
    std::set<gui_base *> children;
    std::vector<gui_base *> children_ordered;

    bool running = false;

    int zorder = 0;
};

}
