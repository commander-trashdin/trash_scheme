import gdb

class PrintUnorderedSet(gdb.Command):
    "A command to print std::unordered_set containing polymorphic objects"

    def __init__(self):
        super(PrintUnorderedSet, self).__init__("print_uset", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        # Split arguments, expect the name of the set variable
        argv = gdb.string_to_argv(arg)
        if len(argv) != 1:
            raise gdb.GdbError('Usage: print_uset variable_name')

        # Access the unordered_set variable
        uset = gdb.parse_and_eval(argv[0])

        # Assuming unordered_set has an internal array of pointers
        # This depends heavily on the implementation details of the std::unordered_set
        buckets = uset['_M_h']['_M_buckets']
        bucket_count = uset['_M_h']['_M_bucket_count']

        for i in range(bucket_count):
            node = buckets[i]
            while node != 0:
                obj_ptr = node['_M_nxt']
                obj = obj_ptr.dereference()
                obj_address = int(obj_ptr.cast(gdb.lookup_type('void').pointer()))
                # Try to dynamically determine the type via vtable (pseudo-code)
                actual_type = determine_type_from_vtable(obj)  # You need to implement this
                # Print the dereferenced object as its actual type
                print('Object at {}: {}'.format(hex(obj_address), gdb.parse_and_eval(f'(({actual_type}*){obj_address})').dereference()))
                node = node['_M_nxt']


def get_type_from_vtable(obj_ptr):
    """
    Extracts the type name from an object's vtable.
    obj_ptr should be a gdb.Value representing a pointer to the object.
    """
    try:
        # Cast obj_ptr to an appropriate pointer type to access its vtable
        void_ptr = obj_ptr.cast(gdb.lookup_type('void').pointer())
        
        # The vtable is typically the first pointer in an object with virtual functions
        vtable_ptr = void_ptr.dereference()
        
        # The typeinfo pointer is typically the first element in the vtable
        # Assuming the offset to typeinfo is at zero, adjust if your ABI differs
        typeinfo_ptr = vtable_ptr.cast(gdb.lookup_type('void').pointer())
        
        # Dereference typeinfo_ptr to access the typeinfo object
        # Offset might be needed to reach the actual typeinfo, depending on ABI
        typeinfo_address = typeinfo_ptr.dereference()
        
        # typeinfo objects have a `__name` field containing the type name
        # Cast typeinfo_address to std::type_info to access this field
        typeinfo_obj = typeinfo_address.cast(gdb.lookup_type('typeinfo').pointer())
        type_name = typeinfo_obj.dereference()['__name'].string()
        
        return type_name
    except Exception as e:
        print(f"Error determining type from vtable: {str(e)}")
        return "unknown"




def determine_type_from_vtable(obj):
    obj_ptr = gdb.parse_and_eval(obj.address)
    return get_type_from_vtable(obj_ptr)


PrintUnorderedSet()
