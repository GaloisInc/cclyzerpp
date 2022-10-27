// Default to empty definitions
#ifndef GROUP_BEGIN
#define GROUP_BEGIN(group)
#endif
#ifndef GROUP_END
#define GROUP_BEGIN(group)
#endif
// TODO(#42): Remove fname, derive from group + name
#ifndef PREDICATE
#define PREDICATE(group, name, fname)
#endif
// The goal of the above TODO is to move most or all of the below declarations
// from using PREDICATE to PREDICATE2S? and PREDICATEIS?.
#define PREDICATE2(group, name) PREDICATE(group, name, group ## _ ## name)
#define PREDICATE2S(group, name) PREDICATE(group, name, group ## name)
#define PREDICATEI(group, name) PREDICATE(group, name, group ## _instr_ ## name)
#define PREDICATEIS(group, name) PREDICATE(group, name, group ## instr_ ## name)

GROUP_BEGIN(block)
PREDICATE2(block, predecessor)
GROUP_END(block)

GROUP_BEGIN(global_var)
PREDICATE(global_var, id, global_var)
PREDICATE2(global_var, name)
PREDICATE2(global_var, demangled_name)
PREDICATE2(global_var, type)
PREDICATE2(global_var, initializer)
PREDICATE2(global_var, section)
PREDICATE2(global_var, aligned_to)
PREDICATE2(global_var, flag)
PREDICATE2(global_var, linkage)
PREDICATE2(global_var, visibility)
PREDICATE2(global_var, threadlocal_mode)
GROUP_END(global_var)

GROUP_BEGIN(alias)
PREDICATE(alias, id, alias)
PREDICATE2(alias, name)
PREDICATE2(alias, type)
PREDICATE2(alias, linkage)
PREDICATE2(alias, visibility)
PREDICATE2(alias, aliasee)
GROUP_END(alias)

GROUP_BEGIN(func)
PREDICATE(func, id_defn, func)
PREDICATE(func, id_decl, func_decl)
PREDICATE2(func, unnamed_addr)
PREDICATE2(func, linkage)
PREDICATE2(func, visibility)
PREDICATE2(func, calling_conv)
PREDICATE2(func, section)
PREDICATE2(func, alignment)
PREDICATE2(func, gc)
PREDICATE2(func, pers_fn)
PREDICATE2(func, name)
PREDICATE2(func, ty)
PREDICATE2(func, signature)
PREDICATE2(func, param)
GROUP_END(function)

GROUP_BEGIN(instr)
PREDICATE2(instr, assigns_to)
PREDICATE2(instr, flag)
PREDICATE2(instr, successor)
PREDICATE2(instr, bb_entry)
PREDICATE2(instr, func)
PREDICATE2(instr, pos)
GROUP_END(instr)

GROUP_BEGIN(unreachable)
PREDICATE2(unreachable, instr)
GROUP_END(unreachable)

GROUP_BEGIN(add)
PREDICATE2(add, instr)
PREDICATEI(add, first_operand)
PREDICATEI(add, second_operand)
GROUP_END(add)

GROUP_BEGIN(fadd)
PREDICATE2(fadd, instr)
PREDICATEI(fadd, first_operand)
PREDICATEI(fadd, second_operand)
GROUP_END(fadd)

GROUP_BEGIN(sub)
PREDICATE2(sub, instr)
PREDICATEI(sub, first_operand)
PREDICATEI(sub, second_operand)
GROUP_END(sub)

GROUP_BEGIN(fsub)
PREDICATE2(fsub, instr)
PREDICATEI(fsub, first_operand)
PREDICATEI(fsub, second_operand)
GROUP_END(fsub)

GROUP_BEGIN(mul)
PREDICATE2(mul, instr)
PREDICATEI(mul, first_operand)
PREDICATEI(mul, second_operand)
GROUP_END(mul)

GROUP_BEGIN(fmul)
PREDICATE2(fmul, instr)
PREDICATEI(fmul, first_operand)
PREDICATEI(fmul, second_operand)
GROUP_END(fmul)

GROUP_BEGIN(udiv)
PREDICATE2(udiv, instr)
PREDICATEI(udiv, first_operand)
PREDICATEI(udiv, second_operand)
GROUP_END(udv)

GROUP_BEGIN(fdiv)
PREDICATE2(fdiv, instr)
PREDICATEI(fdiv, first_operand)
PREDICATEI(fdiv, second_operand)
GROUP_END(fdiv)

GROUP_BEGIN(sdiv)
PREDICATE2(sdiv, instr)
PREDICATEI(sdiv, first_operand)
PREDICATEI(sdiv, second_operand)
GROUP_END(sdiv)

GROUP_BEGIN(urem)
PREDICATE2(urem, instr)
PREDICATEI(urem, first_operand)
PREDICATEI(urem, second_operand)
GROUP_END(urem)

GROUP_BEGIN(srem)
PREDICATE2(srem, instr)
PREDICATEI(srem, first_operand)
PREDICATEI(srem, second_operand)
GROUP_END(srem)

GROUP_BEGIN(frem)
PREDICATE2(frem, instr)
PREDICATEI(frem, first_operand)
PREDICATEI(frem, second_operand)
GROUP_END(frem)

// Bitwise Binary Instrs

GROUP_BEGIN(shl)
PREDICATE2(shl, instr)
PREDICATEI(shl, first_operand)
PREDICATEI(shl, second_operand)
GROUP_END(shl)

GROUP_BEGIN(lshr)
PREDICATE2(lshr, instr)
PREDICATEI(lshr, first_operand)
PREDICATEI(lshr, second_operand)
GROUP_END(lshr)

GROUP_BEGIN(ashr)
PREDICATE2(ashr, instr)
PREDICATEI(ashr, first_operand)
PREDICATEI(ashr, second_operand)
GROUP_END(ashr)

GROUP_BEGIN(and_)
PREDICATE2S(and_, instr)
PREDICATEIS(and_, first_operand)
PREDICATEIS(and_, second_operand)
GROUP_END(and_)

GROUP_BEGIN(or_)
PREDICATE2S(or_, instr)
PREDICATEIS(or_, first_operand)
PREDICATEIS(or_, second_operand)
GROUP_END(or_)

GROUP_BEGIN(xor_)
PREDICATE2S(xor_, instr)
PREDICATEIS(xor_, first_operand)
PREDICATEIS(xor_, second_operand)
GROUP_END(xor_)

// Terminator Instrs

GROUP_BEGIN(ret)
PREDICATE2(ret, instr)
// Can't be named 'void', as that's a C++ keyword.
PREDICATEI(ret, void_)
PREDICATEI(ret, operand)
GROUP_END(ret)

GROUP_BEGIN(br)
PREDICATE2(br, instr)
PREDICATEI(br, cond)
PREDICATEI(br, condition)
PREDICATEI(br, true_label)
PREDICATEI(br, false_label)
PREDICATEI(br, uncond)
PREDICATEI(br, uncond_label)
GROUP_END(br)

GROUP_BEGIN(switch_)
PREDICATE2S(switch_, instr)
PREDICATEIS(switch_, operand)
PREDICATEIS(switch_, default_label)
PREDICATEIS(switch_, case_value)
PREDICATEIS(switch_, case_label)
PREDICATEIS(switch_, ncases)
GROUP_END(switch_)

GROUP_BEGIN(indirectbr)
PREDICATE2(indirectbr, instr)
PREDICATEI(indirectbr, address)
PREDICATEI(indirectbr, label)
PREDICATEI(indirectbr, nlabels)
GROUP_END(indirectbr)

GROUP_BEGIN(resume)
PREDICATE2(resume, instr)
PREDICATEI(resume, operand)
GROUP_END(resume)

GROUP_BEGIN(invoke)
PREDICATE2(invoke, instr)
PREDICATEI(invoke, arg)
PREDICATEI(invoke, func_operand)
PREDICATEI(invoke, calling_conv)
PREDICATEI(invoke, normal_label)
PREDICATEI(invoke, exception_label)
GROUP_END(invoke)

// Vector Operations

GROUP_BEGIN(extractelement)
PREDICATE2(extractelement, instr)
PREDICATEI(extractelement, base)
PREDICATEI(extractelement, index)
GROUP_END(extractelement)

GROUP_BEGIN(insertelement)
PREDICATE2(insertelement, instr)
PREDICATEI(insertelement, base)
PREDICATEI(insertelement, index)
PREDICATEI(insertelement, value)
GROUP_END(insertelement)

GROUP_BEGIN(shufflevector)
PREDICATE2(shufflevector, instr)
PREDICATEI(shufflevector, first_vector)
PREDICATEI(shufflevector, second_vector)
PREDICATEI(shufflevector, mask)
GROUP_END(shufflevector)

// Aggregate Operations

GROUP_BEGIN(extractvalue)
PREDICATE2(extractvalue, instr)
PREDICATEI(extractvalue, base)
PREDICATEI(extractvalue, index)
PREDICATEI(extractvalue, nindices)
GROUP_END(extractvalue)

GROUP_BEGIN(insertvalue)
PREDICATE2(insertvalue, instr)
PREDICATEI(insertvalue, base)
PREDICATEI(insertvalue, value)
PREDICATEI(insertvalue, index)
PREDICATEI(insertvalue, nindices)
GROUP_END(insertvalue)

// Memory Operations

GROUP_BEGIN(alloca)
PREDICATE2(alloca, instr)
PREDICATEI(alloca, size)
PREDICATEI(alloca, alignment)
PREDICATEI(alloca, type)
GROUP_END(alloca)

GROUP_BEGIN(load)
PREDICATE2(load, instr)
PREDICATEI(load, alignment)
PREDICATEI(load, ordering)
PREDICATEI(load, address)
PREDICATEI(load, is_volatile)
GROUP_END(load)

GROUP_BEGIN(store)
PREDICATE2(store, instr)
PREDICATEI(store, alignment)
PREDICATEI(store, ordering)
PREDICATEI(store, value)
PREDICATEI(store, address)
PREDICATEI(store, is_volatile)
GROUP_END(store)

GROUP_BEGIN(fence)
PREDICATE2(fence, instr)
PREDICATEI(fence, ordering)
GROUP_END(fence)

GROUP_BEGIN(atomicrmw)
PREDICATE2(atomicrmw, instr)
PREDICATEI(atomicrmw, ordering)
PREDICATEI(atomicrmw, operation)
PREDICATEI(atomicrmw, address)
PREDICATEI(atomicrmw, value)
PREDICATEI(atomicrmw, is_volatile)
GROUP_END(atomicrmw)

GROUP_BEGIN(cmpxchg)
PREDICATE2(cmpxchg, instr)
PREDICATEI(cmpxchg, ordering)
PREDICATEI(cmpxchg, address)
PREDICATEI(cmpxchg, cmp_value)
PREDICATEI(cmpxchg, new_value)
PREDICATEI(cmpxchg, type)
PREDICATEI(cmpxchg, is_volatile)
GROUP_END(cmpxchg)

GROUP_BEGIN(getelementptr)
PREDICATE2(getelementptr, instr)
PREDICATEI(getelementptr, base)
PREDICATEI(getelementptr, index)
PREDICATEI(getelementptr, nindices)
PREDICATEI(getelementptr, inbounds)
GROUP_END(getelementptr)

// Conversion Operations

GROUP_BEGIN(trunc)
PREDICATE2(trunc, instr)
PREDICATEI(trunc, from_operand)
PREDICATEI(trunc, to_type)
GROUP_END(trunc)

GROUP_BEGIN(zext)
PREDICATE2(zext, instr)
PREDICATEI(zext, from_operand)
PREDICATEI(zext, to_type)
GROUP_END(zext)

GROUP_BEGIN(sext)
PREDICATE2(sext, instr)
PREDICATEI(sext, from_operand)
PREDICATEI(sext, to_type)
GROUP_END(sext)

GROUP_BEGIN(fptrunc)
PREDICATE2(fptrunc, instr)
PREDICATEI(fptrunc, from_operand)
PREDICATEI(fptrunc, to_type)
GROUP_END(fptrunc)

GROUP_BEGIN(fpext)
PREDICATE2(fpext, instr)
PREDICATEI(fpext, from_operand)
PREDICATEI(fpext, to_type)
GROUP_END(fpext)

GROUP_BEGIN(fptoui)
PREDICATE2(fptoui, instr)
PREDICATEI(fptoui, from_operand)
PREDICATEI(fptoui, to_type)
GROUP_END(fptoui)

GROUP_BEGIN(fptosi)
PREDICATE2(fptosi, instr)
PREDICATEI(fptosi, from_operand)
PREDICATEI(fptosi, to_type)
GROUP_END(fptosi)

GROUP_BEGIN(uitofp)
PREDICATE2(uitofp, instr)
PREDICATEI(uitofp, from_operand)
PREDICATEI(uitofp, to_type)
GROUP_END(uitofp)

GROUP_BEGIN(sitofp)
PREDICATE2(sitofp, instr)
PREDICATEI(sitofp, from_operand)
PREDICATEI(sitofp, to_type)
GROUP_END(sitofp)

GROUP_BEGIN(ptrtoint)
PREDICATE2(ptrtoint, instr)
PREDICATEI(ptrtoint, from_operand)
PREDICATEI(ptrtoint, to_type)
GROUP_END(ptrtoint)

GROUP_BEGIN(inttoptr)
PREDICATE2(inttoptr, instr)
PREDICATEI(inttoptr, from_operand)
PREDICATEI(inttoptr, to_type)
GROUP_END(inttoptr)

GROUP_BEGIN(bitcast)
PREDICATE2(bitcast, instr)
PREDICATEI(bitcast, from_operand)
PREDICATEI(bitcast, to_type)
GROUP_END(bitcast)

// Other Operations

GROUP_BEGIN(icmp)
PREDICATE2(icmp, instr)
PREDICATEI(icmp, condition)
PREDICATEI(icmp, first_operand)
PREDICATEI(icmp, second_operand)
GROUP_END(icmp)

GROUP_BEGIN(fcmp)
PREDICATE2(fcmp, instr)
PREDICATEI(fcmp, condition)
PREDICATEI(fcmp, first_operand)
PREDICATEI(fcmp, second_operand)
GROUP_END(fcmp)

GROUP_BEGIN(phi)
PREDICATE2(phi, instr)
PREDICATEI(phi, type)
PREDICATEI(phi, pair_value)
PREDICATEI(phi, pair_label)
PREDICATEI(phi, npairs)
GROUP_END(phi)

GROUP_BEGIN(select)
PREDICATE2(select, instr)
PREDICATEI(select, condition)
PREDICATEI(select, first_operand)
PREDICATEI(select, second_operand)
GROUP_END(select)

GROUP_BEGIN(va_arg)
PREDICATE2(va_arg, instr)
PREDICATEI(va_arg, va_list)
PREDICATEI(va_arg, type)
GROUP_END(va_arg)

GROUP_BEGIN(call)
PREDICATE2(call, instr)
PREDICATEI(call, func_operand)
PREDICATEI(call, arg)
PREDICATEI(call, calling_conv)
PREDICATEI(call, tail_opt)
GROUP_END(call)

GROUP_BEGIN(landingpad)
PREDICATE2(landingpad, instr)
PREDICATEI(landingpad, type)
PREDICATEI(landingpad, catch_clause)
PREDICATEI(landingpad, filter_clause)
PREDICATEI(landingpad, nclauses)
// TODO(#42): Remove is
PREDICATE(landingpad, cleanup, landingpad_instr_is_cleanup)
GROUP_END(landingpad)

// Types

GROUP_BEGIN(primitive_type)
PREDICATE(primitive_type, id, primitive_type)
GROUP_END(primitive_type)

GROUP_BEGIN(integer_type)
PREDICATE(integer_type, id, integer_type)
GROUP_END(integer_type)

GROUP_BEGIN(fp_type)
PREDICATE(fp_type, id, fp_type)
GROUP_END(fp_type)

GROUP_BEGIN(type)
PREDICATE(type, alloc_size, type_has_size)
PREDICATE(type, store_size, type_has_unpadded_size)
GROUP_END(type)

GROUP_BEGIN(func_type)
PREDICATE(func_type, id, func_type)
PREDICATE(func_type, varargs, func_type_has_varargs)
PREDICATE(func_type, return_type, func_type_return)
PREDICATE(func_type, param_type, func_type_param)
PREDICATE(func_type, nparams, func_type_nparams)
GROUP_END(func_type)

GROUP_BEGIN(ptr_type)
PREDICATE(ptr_type, id, pointer_type)
PREDICATE(ptr_type, component_type, pointer_type_has_component)
PREDICATE(ptr_type, addr_space, pointer_type_has_addr_space)
GROUP_END(ptr_type)

GROUP_BEGIN(vector_type)
PREDICATE(vector_type, id, vector_type)
PREDICATE(vector_type, component_type, vector_type_has_component)
PREDICATE(vector_type, size, vector_type_has_size)
GROUP_END(vector_type)

GROUP_BEGIN(array_type)
PREDICATE(array_type, id, array_type)
PREDICATE(array_type, component_type, array_type_has_component)
PREDICATE(array_type, size, array_type_has_size)
GROUP_END(array_type)

GROUP_BEGIN(struct_type)
PREDICATE(struct_type, id, struct_type)
PREDICATE(struct_type, field_type, struct_type_field)
PREDICATE(struct_type, field_offset, struct_type_field_offset)
PREDICATE(struct_type, field_bit_offset, struct_type_field_bit_offset)
PREDICATE(struct_type, nfields, struct_type_nfields)
PREDICATE(struct_type, opaque, opaque_struct_type)
PREDICATE(struct_type, has_name, struct_type_has_name)
GROUP_END(struct_type)

// Variables and constants

GROUP_BEGIN(variable)
PREDICATE(variable, id, variable)
PREDICATE(variable, type, variable_has_type)
PREDICATE(variable, source_name, variable_has_debug_source_name)
PREDICATE(variable, pos, variable_has_debug_decl_pos)
PREDICATE(variable, in_func, variable_in_func_name)
PREDICATE(variable, name, variable_has_name)
GROUP_END(variable)

GROUP_BEGIN(constant)
PREDICATE(constant, id, constant)
PREDICATE(constant, type, constant_has_type)
PREDICATE(constant, value, constant_has_value)
PREDICATE(constant, in_func, constant_in_func_name)
PREDICATE(constant, expr, constant_expression)
PREDICATE(constant, hash, constant_hashes_to)
PREDICATE(constant, to_integer, constant_to_int)
GROUP_END(constant)

// Constant hierarchy

GROUP_BEGIN(integer_constant)
PREDICATE(integer_constant, id, integer_constant)
GROUP_END(integer_constant)

GROUP_BEGIN(fp_constant)
PREDICATE(fp_constant, id, fp_constant)
GROUP_END(fp_constant)

GROUP_BEGIN(nullptr_constant)
PREDICATE(nullptr_constant, id, nullptr_constant)
GROUP_END(nullptr_constant)

GROUP_BEGIN(func_constant)
PREDICATE(func_constant, id, func_constant)
PREDICATE(func_constant, name, func_constant_fn_name)
GROUP_END(func_constant)

GROUP_BEGIN(global_var_constant)
PREDICATE(global_var_constant, id, global_var_constant)
PREDICATE(global_var_constant, name, global_var_constant_name)
GROUP_END(global_var_constant)

GROUP_BEGIN(constant_array)
PREDICATE(constant_array, id, constant_array)
PREDICATE(constant_array, index, constant_array_index)
PREDICATE(constant_array, size, constant_array_has_size)
GROUP_END(constant_array)

GROUP_BEGIN(constant_struct)
PREDICATE(constant_struct, id, constant_struct)
PREDICATE(constant_struct, index, constant_struct_index)
PREDICATE(constant_struct, size, constant_struct_has_size)
GROUP_END(constant_struct)

GROUP_BEGIN(constant_vector)
PREDICATE(constant_vector, id, constant_vector)
PREDICATE(constant_vector, index, constant_vector_index)
PREDICATE(constant_vector, size, constant_vector_has_size)
GROUP_END(constant_vector)

GROUP_BEGIN(constant_expr)
PREDICATE(constant_expr, id, constant_expression)
GROUP_END(constant_expr)

GROUP_BEGIN(bitcast_constant_expr)
PREDICATE(bitcast_constant_expr, id, bitcast_constant_expression)
PREDICATE(bitcast_constant_expr, from_constant, bitcast_constant_expression_from)
GROUP_END(bitcast_constant_expr)

GROUP_BEGIN(inttoptr_constant_expr)
PREDICATE(inttoptr_constant_expr, id, inttoptr_constant_expression)
PREDICATE(inttoptr_constant_expr, from_int_constant, inttoptr_constant_expression_from)
GROUP_END(inttoptr_constant_expr)

GROUP_BEGIN(ptrtoint_constant_expr)
PREDICATE(ptrtoint_constant_expr, id, ptrtoint_constant_expression)
PREDICATE(ptrtoint_constant_expr, from_ptr_constant, ptrtoint_constant_expression_from)
GROUP_END(ptrtoint_constant_expr)

GROUP_BEGIN(gep_constant_expr)
PREDICATE(gep_constant_expr, id, getelementptr_constant_expression)
PREDICATE(gep_constant_expr, base, getelementptr_constant_expression_base)
PREDICATE(gep_constant_expr, index, getelementptr_constant_expression_index)
PREDICATE(gep_constant_expr, nindices, getelementptr_constant_expression_nindices)
GROUP_END(gep_constant_expr)

GROUP_BEGIN(undef_constant)
PREDICATE(undef_constant, id, undef_constant)
GROUP_END(undef_constant)

GROUP_BEGIN(poison_constant)
PREDICATE(poison_constant, id, poison_constant)
GROUP_END(poison_constant)

// Inline Assembly

GROUP_BEGIN(inline_asm)
PREDICATE(inline_asm, id, inline_asm)
PREDICATE(inline_asm, text, inline_asm_text)
PREDICATE(inline_asm, constraints, inline_asm_constraints)
GROUP_END(inline_asm)

// Signatures

GROUP_BEGIN(signature)
PREDICATE2(signature, none)
PREDICATE2(signature, return_alloc)
PREDICATE2(signature, return_alloc_once)
PREDICATE2(signature, return_aliases_arg)
PREDICATE2(signature, return_aliases_arg_reachable)
PREDICATE2(signature, return_points_to_global)
PREDICATE2(signature, return_aliases_global)
PREDICATE2(signature, return_aliases_global_reachable)
PREDICATE2(signature, arg_alloc)
PREDICATE2(signature, arg_alloc_once)
PREDICATE2(signature, arg_memcpy_arg)
PREDICATE2(signature, arg_memcpy_arg_reachable)
PREDICATE2(signature, arg_points_to_global)
PREDICATE2(signature, arg_memcpy_global)
PREDICATE2(signature, arg_memcpy_global_reachable)
PREDICATE2(signature, global_memcpy_arg)
PREDICATE2(signature, global_memcpy_arg_reachable)
GROUP_END(signature)

// User options
GROUP_BEGIN(user)
PREDICATE(user, options, user_options)
GROUP_END(user)

#undef GROUP_BEGIN
#undef GROUP_END
#undef PREDICATE
#undef PREDICATE2
#undef PREDICATE2S
#undef PREDICATEI
#undef PREDICATEIS
