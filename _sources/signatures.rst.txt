.. _signatures:

Signatures
----------

cclyzer++ can model the behavior of external code via *signatures* that describe
the salient features of the code that is not available for analysis. Signatures
are provided as a JSON file to the fact generator. The top level of the JSON
should be a list, and each entry should have the form:

.. code-block::

   {
     "name": "function_name",
     "signatures": [
       {
         "signature_name": [ signature arguments ... ]
       },
       ...
     ]
   }

where ``function_name`` is a regular expression that matches the mangled
(LLVM-level) function name.

If a signature should be applied multiple times with different arguments, it
should appear multiple times in the list. For example:

.. code-block::

   {
     "name": "function_name",
     "signatures": [
       {
         "pts_arg_memcpy_arg": [ 0, 1 ],
         "pts_arg_memcpy_arg": [ 0, 2 ]
       }
     ]
   }

Kinds of Signatures
*******************

:code:`- pts_none: []`

  Used to indicate that the function has no points-to relevant
  behavior and that the function should not be reported as missing
  points-to signatures.

:code:`- pts_return_alloc: []`

  Allocates a new memory object with type compatible with the
  callsite's return type.

  Example:
   :code:`pts_return_alloc: []`

   .. code-block::

      int *a = example();
      int *b = example();

   Variables :code:`a` and :code:`b` will point to distinct allocations of type :code:`int`.

:code:`- pts_return_alloc_once: []`

  The pointer returned by this function at any callsite will point
  to the same allocation. Can be used to model libraries which
  return a pointer to a static internal location.

  Example:
   :code:`pts_return_alloc_once: []`

   .. code-block::

      int *a = example();
      int *b = example();

   Variables :code:`a` and :code:`b` will point to the same allocation of type :code:`int`.

:code:`- pts_return_aliases_arg: [ <arg index> ]`

  The pointer returned by this function may point to any
  type-compatible allocation pointed to by the argument at the
  specified index.

  Example:
   :code:`pts_return_aliases_arg: [ 1 ]`

   .. code-block::

      int a = 0;
      int b = 1;
      int *c = example(&a, &b);

   Variable :code:`c` will point to the stack allocation for variable :code:`b`.

:code:`- pts_return_aliases_arg_reachable: [ <arg index> ]`

  The pointer returned by this function may point to any
  type-compatible allocation that is reachable in the points-to
  graph from the argument at the specified index.

  Can be used to model functions that extract interior pointers
  from arguments.

  Example:
   :code:`pts_return_aliases_arg: [ 1 ]`

   .. code-block::

      struct container { int *internal; };
      int a = 0;
      int b = 1;
      struct container as = { .internal = &a };
      struct container bs = { .internal = &b };
      int *c = example(&as, &bs);

   Variable :code:`c` will point to the stack allocation for variable :code:`b`.

:code:`- pts_return_points_to_global: [ <global name> ]`

  The pointer returned by this function points to the allocation
  corresponding to the named global (which must be defined in the
  program under analysis).

  Example:
   :code:`pts_return_points_to_global: [ test ]`

   .. code-block::

      int test = 5;

      void main(void) {
        int *a = example();
      }

   Variable :code:`a` will point to the global allocation for :code:`test`.

:code:`- pts_return_aliases_global: [ <global name> ]`

  The pointer returned by this function may point to any
  type-compatible allocation pointed to by the named global (which
  must be defined in the program under analysis).

  Example:
   :code:`pts_return_aliases_global: [ testptr ]`

   .. code-block::

      int test = 5;
      int *testptr = &test;

      void main(void) {
        int *a = example();
      }

   Variable :code:`a` will point to the global allocation for :code:`test`.

:code:`- pts_return_aliases_global_reachable: [ <global name> ]`

  The pointer returned by this function may point to any
  type-compatible allocation that is reachable in the points-to
  graph from the named global (which must be defined in the program
  under analysis).

  Example:
   :code:`pts_return_aliases_global_reachable: [ testptr ]`

   .. code-block::

      struct container { int *internal; };

      int test = 5;
      struct container teststruct = { .internal = &test };
      int *testptr = &teststruct;

      void main(void) {
        int *a = example();
      }

  Variable :code:`a` will point to the global allocation for :code:`test`.

:code:`- pts_arg_alloc: [ <arg index> ]`

  Allocates a new memory object with type compatible with the
  specified argument's pointer type and updates the points-to set of
  the pointer.

  Example:
   :code:`pts_arg_alloc: [ 1 ]`

   .. code-block::

      int *a = nullptr;
      int *b = nullptr;
      int *c = nullptr;
      int *d = nullptr;
      example(&a, &b);
      example(&c, &d);

   Variables :code:`b` and :code:`d` will point to distinct
   allocations of type :code:`int`. Variables :code:`a` and :code:`c` will not
   point to any allocations.

:code:`- pts_arg_alloc_once: [ <arg index> ]`

  Any pointers pointed-to by callsite arguments at the specified index
  will point to the same allocation. Can be used to model libraries which
  assign pointers to static locations into output variables.

  Example:
   :code:`pts_arg_alloc_once: [ 1 ]`

   .. code-block::

      int *a = nullptr;
      int *b = nullptr;
      int *c = nullptr;
      int *d = nullptr;
      example(&a, &b);
      example(&c, &d);

   Variables :code:`b` and :code:`d` will point to the same allocation
   of type :code:`int`.

:code:`- pts_arg_memcpy_arg: [ <destination arg index>, <source arg index> ]`

  Points-to sets will be updated as if the memory pointed to by the
  source argument might have been copied to the memory pointed to by
  the destination argument.

  Example:
   :code:`pts_arg_memcpy_arg: [ 0, 1 ]`

   .. code-block::

      int a = 0;
      int b = 1;
      int *ap = &a;
      int *bp = &b;
      example(&ap, &bp);

   Variable :code:`ap` will point to the allocations for both
   variables :code:`a` and :code:`b`. The points-to set of variable
   :code:`bp` will be unchanged and still refer only to :code:`b`.

:code:`- pts_arg_memcpy_arg_reachable: [ <destination arg index>, <source arg index> ]`

  Points-to sets will be updated as if any type-compatible  memory
  allocation reachable from the source argument might have been copied
  to the memory pointed to by the destination argument.

  Example:
   :code:`pts_arg_memcpy_arg_reachable: [ 0, 1 ]`

   .. code-block::

      struct container { int *internal; };
      int a = 0;
      int b = 1;
      struct container sb = {.internal = &b};
      int *ap = &a;
      struct container *sbp = &sb;
      example(&ap, &sbp);

   Variable :code:`ap` will point to the allocations for both
   variables :code:`a` and :code:`b`. The points-to set of variable
   :code:`sbp` and :code:`sb.internal` will be unchanged.

:code:`- pts_arg_memcpy_global: [ <destination arg index>, <global name> ]`

  Points-to sets will be updated as if the named global might have
  been copied to the memory pointed to by the destination argument.

  Example:
   :code:`pts_arg_memcpy_arg: [ 0, test_struct ]`

   .. code-block::

      struct container { int *internal; };
      test_int = 0;
      struct container test_struct = {.internal = &global_int};

      void main(void) {
        struct container a;
        example(&a);
      }

   Variable :code:`a.internal` will point to the global allocation
   :code:`test_int`.

:code:`- pts_arg_memcpy_global_reachable: [ <destination arg index>, <global name> ]`

  Points-to sets will be updated as if the named global or any data
  reachable from it might have been copied to the memory pointed to by
  the destination argument.

  Example:
   :code:`pts_arg_memcpy_global_reachable: [ 0, test_struct ]`

   .. code-block::


      struct container { int *internal; };
      test_int = 0;
      struct container test_struct = {.internal = &global_int};

      void main(void) {
        int a = 0;
        int *ap = &a;
        example(&ap);
      }

   Variable :code:`ap` will point to the global allocation :code:`test_int`.

:code:`- pts_arg_points_to_global: [ <destination arg index>, <global name> ]`

  The points-to set of the pointer pointed to by the specified
  argument will be updated to include the allocation corresponding to
  the named global.

  Example:
   :code:`pts_arg_points_to_global: [ 0, test_int ]`

   .. code-block::

      test_int = 0;

      void main(void) {
        int a = 0;
        int *ap = &a;
        example(&ap);
      }

   Variable :code:`ap` will point to the global alocation :code:`test_int`.

Examples
********

The libc function :code:`close` has no visible effect on memory. It can be
modeled as

.. code-block::

   {"name": "^close$", "signatures": [{"pts_none": []}]}

``fgetc(3)`` says:

.. code-block::

  char *fgets(char *s, int size, FILE *stream)

  gets() and fgets() return s on success, [...]

This behavior can be modeled by the following signature:

.. code-block::

   {"name": "^fgets$", "signatures": [{"pts_return_aliases_arg": [0]}]}
