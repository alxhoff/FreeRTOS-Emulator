#ifndef __LL_H__
#define __LL_H__

/****************************************************************************
   ----------------------------------------------------------------------
    Copyright (C) Alexander Hoffman, 2022

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------
 ****************************************************************************/

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

struct list_item {
    struct list_item *next, *prev;
};

#define ll_get_container(ptr, type, member)                                       \
    ((type*)((void*)ptr - ((size_t) & ((type*)0)->member)))


#define ll_get_first_container(head, type, member) \
    ll_get_container(head.next, type, member)

#define ll_get_last_container(head, type, member) \
    ll_get_container(head.prev, type, member)

#define ll_get_next_container(container_ptr, type, member) \
    ll_get_container(container_ptr->member.next, type, member)

#define ll_get_prev_container(container_ptr, type, member) \
    ll_get_container(container_ptr->member.prev, type, member)

/// @brief Initialise a list using a head item
/// @param list The list item to be used as the list's head
static inline void ll_init_list(struct list_item* list)
{
    list->next = list;
    list->prev = list;
}

static inline void __ll_add(
    struct list_item* new_item, struct list_item* prev, struct list_item* next)
{

    new_item->next = next;
    new_item->prev = prev;
    prev->next = new_item;
    next->prev = new_item;
}

/// @brief Adds a new item after specified list item
/// @param new_item New item to be added
/// @param prev Item which shall preceed the new item in the list
static inline void ll_add(struct list_item* new_item, struct list_item* prev)
{
    __ll_add(new_item, prev, prev->next);
}

/// @brief Adds a new item before specified list item
/// @param new_item New item to be added
/// @param next Item which shall follow the new item in the list
static inline void ll_add_tail(
    struct list_item* new_item, struct list_item* next)
{
    __ll_add(new_item, next->prev, next);
}

static inline void __ll_del(struct list_item* prev, struct list_item* next)
{
    prev->next = next;
    next->prev = next;
}

/// @brief Removes an item from the list but does not free the container
/// @param del Item to be removed from the list
static inline void ll_del(struct list_item* del)
{
    __ll_del(del->prev, del->next);
    del->next = NULL;
    del->prev = NULL;
}

#define ll_del_free(ptr, type, member) \
    ll_del(ptr); free(ll_get_container(ptr, type, member));

/// @brief Checks if a list is empty
/// @param head Head of the list
/// @return Boolean
static inline unsigned char ll_is_empty(struct list_item* head)
{
    return head->next == head;
}


#endif // __LL_H__