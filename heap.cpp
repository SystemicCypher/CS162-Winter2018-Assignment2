#include "heap.hpp"

#include <iostream>
#include <map>


Heap::Heap(int32_t heap_size) : heap_size(heap_size), root_set() {
  heap = new byte[heap_size];
  from = heap;
  to = heap + heap_size / 2;
  bump_ptr = 0;
}

Heap::~Heap() {
  delete[] heap;
}

// This method should print out the state of the heap.
// It's not mandatory to implement it, but would be a very useful
// debugging tool. It's called whenever the DEBUG command is found
// in the input program.
void Heap::debug() {
  std::cout<<"memory?\n";
}

// The allocate method allocates a chunk of memory of given size.
// It returns an object pointer, which is local to the from space.
// For example, the first allocated object would have the address 0.
// IMPORTANT: This method should initiate garbage collection when there is not
// enough memory. If there is still insufficient memory after garbage collection,
// this method should throw an out_of_memory exception.
obj_ptr Heap::allocate(int32_t size) {
  if(bump_ptr + size > heap_size/2){
    collect();
    //debug();
  }
  if(bump_ptr + size > heap_size/2){
    OutOfMemoryException::OutOfMemoryException();
    return 0;
  }
  obj_ptr allocatedStart = bump_ptr;
  bump_ptr += size;
  return allocatedStart;
}

void Heap::objCopy(obj_ptr& toDup, byte* alloc, int32_t& bump){
  if(forward.count(toDup) == 1){
      toDup = local_address(forward[toDup]);
  }
  else{
    auto type = get_object_type(toDup);
    size_t size;
    void* dexter = NULL;
    switch(type) {
      case FOO: {
        size = sizeof(Foo);
        dexter = global_address<Foo>(toDup);
        break;
      }
      case BAR: {
        size = sizeof(Bar);
        dexter = global_address<Bar>(toDup);
        break;
      }
      case BAZ: {
        size = sizeof(Baz);
        dexter = global_address<Baz>(toDup);
        break;
      }
    }
    memcpy(alloc, dexter, size);
    forward[toDup] = alloc;
    alloc += size;
    toDup = bump;
    bump += size;
  }
}

// This method should implement the actual semispace garbage collection.
// As a final result this method *MUST* call print();
void Heap::collect() {
  bump_ptr = heap_size/2; //heap-local pointer 0 to end of fromspace. Can't modify the object.
  auto allocptr = to; //global pointer, actual memory address. Can modify the object.
  auto scanptr = to; //global pointer to get the subtree stuff


  for(auto& root : root_set){
    objCopy(root.second, allocptr, bump_ptr);
  }

  int32_t scanbump_ptr = heap_size/2;
  while(scanptr < allocptr){
    void* scanned = NULL;
    auto type = get_object_type(scanbump_ptr);
    size_t size;
    switch(type) {
      case FOO: {
        size = sizeof(Foo);
        scanned = global_address<Foo>(scanbump_ptr);
        break;
      }
      case BAR: {
        size = sizeof(Bar);
        scanned = global_address<Bar>(scanbump_ptr);
        break;
      }
      case BAZ: {
        size = sizeof(Baz);
        scanned = global_address<Baz>(scanbump_ptr);
        break;
      }
    }

    if(type == FOO){
      Foo* scannedF = static_cast<Foo*>(scanned);
      objCopy(scannedF->c, allocptr, bump_ptr);
      objCopy(scannedF->d, allocptr, bump_ptr);
    }
    else if(type == BAR){
      Bar* scannedB = static_cast<Bar*>(scanned);
      objCopy(scannedB->c, allocptr, bump_ptr);
      objCopy(scannedB->f, allocptr, bump_ptr);
    }
    else if(type == BAZ){
      Baz* scannedZ = static_cast<Baz*>(scanned);
      objCopy(scannedZ->b, allocptr, bump_ptr);
      objCopy(scannedZ->c, allocptr, bump_ptr);
    }

    scanptr += size;
    scanbump_ptr += size;
  }

  //swapping
  auto temp = from;
  to = from;
  from = temp;
  // Please do not remove the call to print, it has to be the final
  // operation in the method for your assignment to be graded.
  print();
}

obj_ptr Heap::get_root(const std::string& name) {
  auto root = root_set.find(name);
  if(root == root_set.end()) {
    throw std::runtime_error("No such root: " + name);
  }

  return root->second;
}

object_type Heap::get_object_type(obj_ptr ptr) {
  return *reinterpret_cast<object_type*>(from + ptr);
}

// Finds fields by path / name; used by get() and set().
obj_ptr *Heap::get_nested(const std::vector<std::string>& path) {
  obj_ptr init = get_root(path[0]);
  obj_ptr *fld = &init;

  for(int i = 1; i < path.size(); ++i) {
    auto addr = *fld;
    auto type = *reinterpret_cast<object_type*>(global_address<object_type>(addr));
    auto seg  = path[i];

    switch(type) {
    case FOO: {
      auto *foo = global_address<Foo>(addr);
      if(seg == "c") fld = &foo->c;
      else if(seg == "d") fld = &foo->d;
      else throw std::runtime_error("No such field: Foo." + seg);
      break;
    }
    case BAR: {
      auto *bar = global_address<Bar>(addr);
      if(seg == "c") fld = &bar->c;
      else if(seg == "f") fld = &bar->f;
      else throw std::runtime_error("No such field: Bar." + seg);
      break;
    }
    case BAZ: {
      auto *baz = global_address<Baz>(addr);
      if(seg == "b") fld = &baz->b;
      else if(seg == "c") fld = &baz->c;
      else throw std::runtime_error("No such field: Baz." + seg);
      break;
    }}
  }

  return fld;
}

obj_ptr Heap::get(const std::vector<std::string>& path) {
  if(path.size() == 1) {
    return get_root(path[0]);
  }
  else {
    return *get_nested(path);
  }
}

void Heap::set(const std::vector<std::string>& path, obj_ptr value) {
  if(path.size() == 1) {
    if(value < 0) root_set.erase(path[0]);
    else root_set[path[0]] = value;
  }
  else {
    *get_nested(path) = value;
  }
}

obj_ptr Heap::new_foo() {
  auto heap_addr = allocate(sizeof(Foo));
  new (from + heap_addr) Foo(object_id++);
  return heap_addr;
}

obj_ptr Heap::new_bar() {
  auto heap_addr = allocate(sizeof(Bar));
  new (from + heap_addr) Bar(object_id++);
  return heap_addr;
}

obj_ptr Heap::new_baz() {
  auto heap_addr = allocate(sizeof(Baz));
  new (from + heap_addr) Baz(object_id++);
  return heap_addr;
}

void Heap::print() {
  byte *position = from;
  std::map<int32_t, const char*> objects;

  while(position < (from + heap_size / 2) && position < (from + bump_ptr)) {
    object_type type = *reinterpret_cast<object_type*>(position);
    switch(type) {
      case FOO: {
        auto obj = reinterpret_cast<Foo*>(position);
        objects[obj->id] = "Foo";
        position += sizeof(Foo);
        break;
      }
      case BAR: {
        auto obj = reinterpret_cast<Bar*>(position);
        objects[obj->id] = "Bar";
        position += sizeof(Bar);
        break;
      }
      case BAZ: {
        auto obj = reinterpret_cast<Baz*>(position);
        objects[obj->id] = "Baz";
        position += sizeof(Baz);
        break;
      }
    }
  }

  std::cout << "Objects in from-space:\n";
  for(auto const& itr: objects) {
    std::cout << " - " << itr.first << ':' << itr.second << '\n';
  }
}
