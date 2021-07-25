#ifndef EHM
#define EHM
// ExpandableHashMap.h
// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions : constructor, destructor, reset,size, associate, find


#include <iostream>
using namespace std;

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
	ExpandableHashMap(double maximumLoadFactor = 0.5); // Constructor max load factor of 0.5 and 8 number of buckets
	~ExpandableHashMap(); //destructor; deletes all of the items in the hashmap
	void reset();		  // resets the hashmap back to 8 buckets, deletes all items
	int size() const;     // return the number of associations in the hashmap
	void associate(const KeyType& key, const ValueType& value);
	// for a map that can't be modified, return a pointer to const ValueType
	const ValueType* find(const KeyType& key) const;
	// DON'T TOUCH THIS
	// for a modifiable map, return a pointer to modifiable ValueType
	ValueType* find(const KeyType& key)
	{
		return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
	}
	// DON'T TOUCH THIS
	// C++11 syntax for preventing copying and assignment
	ExpandableHashMap(const ExpandableHashMap&) = delete;
	ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;
private:
	// Hashmap - Chained Singly Linked List Implementation
	struct Node
	{
		KeyType m_key; // generic key
		ValueType m_val; // generic value
		Node* next; // next node in the linked list
		// Linked List Constructor instead of setting the values and keys with single code statements
		Node(KeyType key, ValueType value)
			:m_key(key), m_val(value), next(nullptr) {}
	};
	Node** d_buckets; // 'd' representing dynamic memory allocation. This is essentially the hashmap
	// Each bucket composed of a linked list ~ the d_bucket is dynamically allocated array
	// of pointers to Node pointers, where each Node pointer is a linked list
	unsigned int h_buckets; // number of buckets
	int nItems; // number of associations
	double maxLoadFactor; // max load factor
	// Helper functions
	void clearHash(); // use in destructor and reset
	double getMaxLoadFactor() const; // passed in parameter for constructor; default is 0.5
	double getLoadFactor() const; // get current load factor
	unsigned int getBucketNum(const KeyType& key, unsigned int h_buckets) const; // Clue from spec: get bucket
	void backToOriginal();
	//void fill(Node **fill, unsigned int buckets);
};


/*--------------------------------------------------Constructor------------------------------------------------*/
template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor)
	:h_buckets(8), nItems(0) // newly constructed ExpandableHashMap must have 8 buckets
{
	if (maximumLoadFactor < 0)
		maxLoadFactor = 0.5;
	else
		maxLoadFactor = maximumLoadFactor;
	d_buckets = new Node * [h_buckets];
	for (unsigned int i = 0; i < h_buckets; i++)  // fill the constructed hashmap with nullptrs
	{
		d_buckets[i] = nullptr;
	}
} // maximum load factor will be 0.5


/*--------------------------------------------------Destructor------------------------------------------------*/
template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
	clearHash(); // call helper function
}

/*----------------------------------------------------Reset----------------------------------------------------*/
template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
	clearHash();
	backToOriginal();
}

/*----------------------------------------------------Size-----------------------------------------------------*/
template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
	return nItems;
}

/*--------------------------------------------------Associate---------------------------------------------------*/
template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
	// For every type of key that your program will use for an ExpandableHashMap you must
	// define an overloaded hash function named hash()
	/*----------------------------------Implementation-----------------------------------*/
	int index = getBucketNum(key, h_buckets);
	/*----Key already exist ~ Update in order to not have duplicate keys-----*/
	if (find(key) != nullptr) //if key is not found
	{
		//cerr << "update" << endl;
		*(find(key)) = value; //decerement on left hand side to change the actual content('find' returns pointer to value associated with a key) 
		return; // return immediately in case to not increment nItems	
	}
	/*------------------Key does not exist, so add it-------------------------*/
	else
	{
		//cerr << "add" << endl;
		Node* n = new Node(key, value);
		n->next = d_buckets[index];									// add top prevent collision
		d_buckets[index] = n;
		nItems++; // increment number of items~size() in hash table
	}
	/// Important special case: • Rehash old to new , replace old with new, free old map
	//
	if (getLoadFactor() > getMaxLoadFactor()) 
	{
		unsigned int twiceBucket = 2 * h_buckets;
		Node** newMap = new Node * [twiceBucket];
		for (unsigned int i = 0; i < twiceBucket; i++) // fill the new hashmap with nullptrs
			newMap[i] = nullptr;
		for (unsigned int i = 0; i < h_buckets; i++)
		{
			Node* toAdd= d_buckets[i];
			while (toAdd != nullptr)
			{
				int bucketIndex = getBucketNum(toAdd->m_key, twiceBucket);
			
				Node* addNew = new Node(toAdd->m_key, toAdd->m_val); // avoiding collisions
				addNew->next = newMap[bucketIndex];
				newMap[bucketIndex] = addNew;

				Node* addnext = toAdd->next;
				delete toAdd;
				toAdd = addnext;
			}
		}
		delete[] d_buckets;		
		d_buckets = newMap;
		h_buckets = twiceBucket;
	}
}

/*----------------------------------------------------Find------------------------------------------------------*/
template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
	int bucketIndex = getBucketNum(key, h_buckets);
	Node* searchMe = d_buckets[bucketIndex];
	if (searchMe != nullptr) // avoid invalidated pointer error
	{
		while (searchMe != nullptr)
		{
			if (searchMe->m_key == key)
				return  &(searchMe->m_val);									// If key is found, return a pointer to the value associated with that key
																			// use ampersand & because d_buckets is a double pointer
			searchMe = searchMe->next; // iterate to next node
		}
	}
	return nullptr;
}


/*-----------------------------------------------Helper Functions----------------------------------------------*/

/*-----ClearHash-----*/
template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::clearHash()
{
	/*
	// iterate through linked list and delete individual allocated nodes
	for (unsigned int i = 0; i < h_buckets; i++)
	{
		Node* temp = d_buckets[i];
		while (temp != nullptr)
		{
			Node* killMe = temp;
			temp = temp->next;
			delete killMe;
		}
	}
	delete[] d_buckets; // delete dynamic array
	// call this function inside 1)destructor and 2)reset 
	*/
	for (unsigned int i = 0; i < h_buckets; i++)
	{
		while (d_buckets[i] != nullptr)
		{
			Node* n = d_buckets[i]->next;
			delete d_buckets[i];
			d_buckets[i] = n;
		}
	}
	delete[] d_buckets; // delete dynamic array
}

/*-----GetLoadFactor----*/
template<typename KeyType, typename ValueType>
double ExpandableHashMap<KeyType, ValueType>::getLoadFactor() const
{
	return (static_cast<float>(size()) / h_buckets);
}

/*-----GetMaxLoadFactor----*/
template<typename KeyType, typename ValueType>
double ExpandableHashMap<KeyType, ValueType>::getMaxLoadFactor() const
{
	return maxLoadFactor;
}

/*----Get Bucket Number--*/
template<typename KeyType, typename ValueType>
unsigned int ExpandableHashMap<KeyType, ValueType>::getBucketNum(const KeyType& key, unsigned int h_buckets) const
{
	unsigned int hasher(const KeyType &k);
	unsigned int bucketNum = hasher(key) % h_buckets;
	return bucketNum;
}

/*---BackToOriginal-----*/
template<typename KeyType, typename ValueType>
void  ExpandableHashMap<KeyType, ValueType>::backToOriginal()
{
	nItems = 0;
	h_buckets = 8;
	d_buckets = new Node * [h_buckets]; // 8 buckets
	for (unsigned int i = 0; i < h_buckets; i++) // fill the new hashmap with nullptrs
		d_buckets[i] = nullptr;
	//fill(d_buckets, h_buckets)
}
/*
template<typename KeyType, typename ValueType>
void  ExpandableHashMap<KeyType, ValueType>::fill(Node **fill, unsigned int buckets)
{
	for (unsigned int i = 0; i < buckets; i++) // fill the new hashmap with nullptrs
		fill[i] = nullptr;
}
*/
#endif EHM