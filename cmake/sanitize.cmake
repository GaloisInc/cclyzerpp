function(address_sanitizer target)
  target_compile_options(${target} PUBLIC -fsanitize=address)
  target_link_libraries(${target} PUBLIC -fsanitize=address)
endfunction()

function(undefined_behavior_sanitizer target)
  target_compile_options(${target} PUBLIC -fsanitize=undefined)
  target_link_libraries(${target} PUBLIC -fsanitize=undefined)
endfunction()