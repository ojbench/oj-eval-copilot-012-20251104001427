/**
 * implement a container like std::linked_hashmap
 */
#ifndef SJTU_LINKEDHASHMAP_HPP
#define SJTU_LINKEDHASHMAP_HPP

// only for std::equal_to<T> and std::hash<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {
    /**
     * In linked_hashmap, iteration ordering is differ from map,
     * which is the order in which keys were inserted into the map.
     * You should maintain a doubly-linked list running through all
     * of its entries to keep the correct iteration order.
     *
     * Note that insertion order is not affected if a key is re-inserted
     * into the map.
     */
    
template<
	class Key,
	class T,
	class Hash = std::hash<Key>, 
	class Equal = std::equal_to<Key>
> class linked_hashmap {
public:
	/**
	 * the internal type of data.
	 * it should have a default constructor, a copy constructor.
	 * You can use sjtu::linked_hashmap as value_type by typedef.
	 */
	typedef pair<const Key, T> value_type;

private:
	struct Node {
		value_type *data;
		Node *prev;
		Node *next;
		Node *hash_next;
		
		Node() : data(nullptr), prev(nullptr), next(nullptr), hash_next(nullptr) {}
		Node(const value_type &val) : data(new value_type(val)), prev(nullptr), next(nullptr), hash_next(nullptr) {}
		~Node() { if (data) delete data; }
	};
	
	Node *head;
	Node *tail;
	Node **buckets;
	size_t bucket_count;
	size_t element_count;
	Hash hash_func;
	Equal equal_func;
	
	static const size_t INITIAL_BUCKET_COUNT = 16;
	static constexpr double MAX_LOAD_FACTOR = 0.75;
	
	void init_buckets(size_t count) {
		bucket_count = count;
		buckets = new Node*[bucket_count];
		for (size_t i = 0; i < bucket_count; ++i) {
			buckets[i] = nullptr;
		}
	}
	
	void rehash(size_t new_bucket_count) {
		Node **old_buckets = buckets;
		size_t old_bucket_count = bucket_count;
		
		init_buckets(new_bucket_count);
		
		Node *curr = head->next;
		while (curr != tail) {
			size_t idx = hash_func(curr->data->first) % bucket_count;
			curr->hash_next = buckets[idx];
			buckets[idx] = curr;
			curr = curr->next;
		}
		
		delete[] old_buckets;
	}
	
	void check_and_rehash() {
		if (element_count > bucket_count * MAX_LOAD_FACTOR) {
			rehash(bucket_count * 2);
		}
	}
	
	Node* find_node(const Key &key) const {
		size_t idx = hash_func(key) % bucket_count;
		Node *curr = buckets[idx];
		while (curr) {
			if (equal_func(curr->data->first, key)) {
				return curr;
			}
			curr = curr->hash_next;
		}
		return nullptr;
	}
 
public:
	/**
	 * see BidirectionalIterator at CppReference for help.
	 *
	 * if there is anything wrong throw invalid_iterator.
	 *     like it = linked_hashmap.begin(); --it;
	 *       or it = linked_hashmap.end(); ++end();
	 */
	class const_iterator;
	class iterator {
	private:
		/**
		 * TODO add data members
		 *   just add whatever you want.
		 */
		Node *node;
		const linked_hashmap *map;
		
		friend class linked_hashmap;
		friend class const_iterator;
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = typename linked_hashmap::value_type;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::output_iterator_tag;

		iterator() : node(nullptr), map(nullptr) {}
		iterator(Node *n, const linked_hashmap *m) : node(n), map(m) {}
		iterator(const iterator &other) : node(other.node), map(other.map) {}
		
		iterator operator++(int) {
			if (!node || node == map->tail) throw invalid_iterator();
			iterator temp = *this;
			node = node->next;
			return temp;
		}
		
		iterator & operator++() {
			if (!node || node == map->tail) throw invalid_iterator();
			node = node->next;
			return *this;
		}
		
		iterator operator--(int) {
			if (!node || node->prev == map->head) throw invalid_iterator();
			iterator temp = *this;
			node = node->prev;
			return temp;
		}
		
		iterator & operator--() {
			if (!node || node->prev == map->head) throw invalid_iterator();
			node = node->prev;
			return *this;
		}
		
		value_type & operator*() const {
			return *(node->data);
		}
		
		bool operator==(const iterator &rhs) const {
			return node == rhs.node && map == rhs.map;
		}
		
		bool operator==(const const_iterator &rhs) const {
			return node == rhs.node && map == rhs.map;
		}
		
		bool operator!=(const iterator &rhs) const {
			return !(*this == rhs);
		}
		
		bool operator!=(const const_iterator &rhs) const {
			return !(*this == rhs);
		}

		value_type* operator->() const noexcept {
			return node->data;
		}
	};
 
	class const_iterator {
		private:
			Node *node;
			const linked_hashmap *map;
			
			friend class linked_hashmap;
		public:
			using difference_type = std::ptrdiff_t;
			using value_type = typename linked_hashmap::value_type;
			using pointer = value_type*;
			using reference = value_type&;
			using iterator_category = std::output_iterator_tag;
			
			const_iterator() : node(nullptr), map(nullptr) {}
			const_iterator(Node *n, const linked_hashmap *m) : node(n), map(m) {}
			const_iterator(const const_iterator &other) : node(other.node), map(other.map) {}
			const_iterator(const iterator &other) : node(other.node), map(other.map) {}
			
			const_iterator operator++(int) {
				if (!node || node == map->tail) throw invalid_iterator();
				const_iterator temp = *this;
				node = node->next;
				return temp;
			}
			
			const_iterator & operator++() {
				if (!node || node == map->tail) throw invalid_iterator();
				node = node->next;
				return *this;
			}
			
			const_iterator operator--(int) {
				if (!node || node->prev == map->head) throw invalid_iterator();
				const_iterator temp = *this;
				node = node->prev;
				return temp;
			}
			
			const_iterator & operator--() {
				if (!node || node->prev == map->head) throw invalid_iterator();
				node = node->prev;
				return *this;
			}
			
			const value_type & operator*() const {
				return *(node->data);
			}
			
			bool operator==(const iterator &rhs) const {
				return node == rhs.node && map == rhs.map;
			}
			
			bool operator==(const const_iterator &rhs) const {
				return node == rhs.node && map == rhs.map;
			}
			
			bool operator!=(const iterator &rhs) const {
				return !(*this == rhs);
			}
			
			bool operator!=(const const_iterator &rhs) const {
				return !(*this == rhs);
			}

			const value_type* operator->() const noexcept {
				return node->data;
			}
	};
 
	/**
	 * TODO two constructors
	 */
	linked_hashmap() : head(new Node()), tail(new Node()), element_count(0) {
		head->next = tail;
		tail->prev = head;
		init_buckets(INITIAL_BUCKET_COUNT);
	}
	
	linked_hashmap(const linked_hashmap &other) : head(new Node()), tail(new Node()), element_count(0) {
		head->next = tail;
		tail->prev = head;
		init_buckets(other.bucket_count);
		
		Node *curr = other.head->next;
		while (curr != other.tail) {
			insert(*(curr->data));
			curr = curr->next;
		}
	}
 
	/**
	 * TODO assignment operator
	 */
	linked_hashmap & operator=(const linked_hashmap &other) {
		if (this == &other) return *this;
		clear();
		
		Node *curr = other.head->next;
		while (curr != other.tail) {
			insert(*(curr->data));
			curr = curr->next;
		}
		return *this;
	}
 
	/**
	 * TODO Destructors
	 */
	~linked_hashmap() {
		clear();
		delete head;
		delete tail;
		delete[] buckets;
	}
 
	/**
	 * TODO
	 * access specified element with bounds checking
	 * Returns a reference to the mapped value of the element with key equivalent to key.
	 * If no such element exists, an exception of type `index_out_of_bound'
	 */
	T & at(const Key &key) {
		Node *node = find_node(key);
		if (!node) throw index_out_of_bound();
		return node->data->second;
	}
	
	const T & at(const Key &key) const {
		Node *node = find_node(key);
		if (!node) throw index_out_of_bound();
		return node->data->second;
	}
 
	/**
	 * TODO
	 * access specified element 
	 * Returns a reference to the value that is mapped to a key equivalent to key,
	 *   performing an insertion if such key does not already exist.
	 */
	T & operator[](const Key &key) {
		Node *node = find_node(key);
		if (node) return node->data->second;
		
		value_type val(key, T());
		auto result = insert(val);
		return result.first->second;
	}
 
	/**
	 * behave like at() throw index_out_of_bound if such key does not exist.
	 */
	const T & operator[](const Key &key) const {
		return at(key);
	}
 
	/**
	 * return a iterator to the beginning
	 */
	iterator begin() {
		return iterator(head->next, this);
	}
	
	const_iterator cbegin() const {
		return const_iterator(head->next, this);
	}
 
	/**
	 * return a iterator to the end
	 * in fact, it returns past-the-end.
	 */
	iterator end() {
		return iterator(tail, this);
	}
	
	const_iterator cend() const {
		return const_iterator(tail, this);
	}
 
	/**
	 * checks whether the container is empty
	 * return true if empty, otherwise false.
	 */
	bool empty() const {
		return element_count == 0;
	}
 
	/**
	 * returns the number of elements.
	 */
	size_t size() const {
		return element_count;
	}
 
	/**
	 * clears the contents
	 */
	void clear() {
		Node *curr = head->next;
		while (curr != tail) {
			Node *next = curr->next;
			delete curr;
			curr = next;
		}
		head->next = tail;
		tail->prev = head;
		element_count = 0;
		for (size_t i = 0; i < bucket_count; ++i) {
			buckets[i] = nullptr;
		}
	}
 
	/**
	 * insert an element.
	 * return a pair, the first of the pair is
	 *   the iterator to the new element (or the element that prevented the insertion), 
	 *   the second one is true if insert successfully, or false.
	 */
	pair<iterator, bool> insert(const value_type &value) {
		Node *existing = find_node(value.first);
		if (existing) {
			return pair<iterator, bool>(iterator(existing, this), false);
		}
		
		check_and_rehash();
		
		Node *new_node = new Node(value);
		
		new_node->prev = tail->prev;
		new_node->next = tail;
		tail->prev->next = new_node;
		tail->prev = new_node;
		
		size_t idx = hash_func(value.first) % bucket_count;
		new_node->hash_next = buckets[idx];
		buckets[idx] = new_node;
		
		++element_count;
		return pair<iterator, bool>(iterator(new_node, this), true);
	}
 
	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
	void erase(iterator pos) {
		if (pos.map != this || pos.node == tail || pos.node == head) {
			throw invalid_iterator();
		}
		
		Node *node = pos.node;
		
		node->prev->next = node->next;
		node->next->prev = node->prev;
		
		size_t idx = hash_func(node->data->first) % bucket_count;
		Node **curr_ptr = &buckets[idx];
		while (*curr_ptr) {
			if (*curr_ptr == node) {
				*curr_ptr = node->hash_next;
				break;
			}
			curr_ptr = &((*curr_ptr)->hash_next);
		}
		
		delete node;
		--element_count;
	}
 
	/**
	 * Returns the number of elements with key 
	 *   that compares equivalent to the specified argument,
	 *   which is either 1 or 0 
	 *     since this container does not allow duplicates.
	 */
	size_t count(const Key &key) const {
		return find_node(key) ? 1 : 0;
	}
 
	/**
	 * Finds an element with key equivalent to key.
	 * key value of the element to search for.
	 * Iterator to an element with key equivalent to key.
	 *   If no such element is found, past-the-end (see end()) iterator is returned.
	 */
	iterator find(const Key &key) {
		Node *node = find_node(key);
		return node ? iterator(node, this) : end();
	}
	
	const_iterator find(const Key &key) const {
		Node *node = find_node(key);
		return node ? const_iterator(node, this) : cend();
	}
};

}

#endif
