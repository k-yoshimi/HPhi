# include guard
if(${CMAKE_PROJECT_NAME} STREQUAL "Project")
  message(FATAL_ERROR "cmake should not be executed directly in subdirectory.")
endif(${CMAKE_PROJECT_NAME} STREQUAL "Project")

add_executable(
  unit_bitcalc
  test_bitcalc.c
  ${PROJECT_SOURCE_DIR}/src/bitcalc.c
  ${PROJECT_SOURCE_DIR}/src/ErrorMessage.c
  ${PROJECT_SOURCE_DIR}/src/global.c
)
target_include_directories(unit_bitcalc PRIVATE ${PROJECT_SOURCE_DIR}/src/include)
target_compile_definitions(unit_bitcalc PRIVATE DSFMT_MEXP=19937)
find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
  target_link_libraries(unit_bitcalc PRIVATE ${MATH_LIBRARY})
endif()

add_test(
  NAME unit_bitcalc
  COMMAND unit_bitcalc
)
set_tests_properties(unit_bitcalc PROPERTIES LABELS "unit")

add_executable(
  unit_setmemory
  test_setmemory.c
  ${PROJECT_SOURCE_DIR}/src/common/setmemory.c
)
target_include_directories(unit_setmemory PRIVATE ${PROJECT_SOURCE_DIR}/src/common)

add_test(
  NAME unit_setmemory
  COMMAND unit_setmemory
)
set_tests_properties(unit_setmemory PROPERTIES LABELS "unit")

add_test(
  NAME unit_readdef_characterization
  COMMAND sh ${CMAKE_SOURCE_DIR}/test/unit/readdef_characterization.sh
)
set_tests_properties(unit_readdef_characterization PROPERTIES LABELS "unit")

add_test(
  NAME unit_readdef_pair_excitation_count_mismatch
  COMMAND sh ${CMAKE_SOURCE_DIR}/test/unit/readdef_pair_excitation_count_mismatch.sh
)
set_tests_properties(unit_readdef_pair_excitation_count_mismatch PROPERTIES LABELS "unit")

add_test(
  NAME unit_readdef_teonebody_count_mismatch
  COMMAND sh ${CMAKE_SOURCE_DIR}/test/unit/readdef_teonebody_count_mismatch.sh
)
set_tests_properties(unit_readdef_teonebody_count_mismatch PROPERTIES LABELS "unit")

add_test(
  NAME unit_readdef_missing_ncond_error
  COMMAND sh ${CMAKE_SOURCE_DIR}/test/unit/readdef_missing_ncond_error.sh
)
set_tests_properties(unit_readdef_missing_ncond_error PROPERTIES LABELS "unit")

add_test(
  NAME unit_sz_characterization
  COMMAND sh ${CMAKE_SOURCE_DIR}/test/unit/sz_characterization.sh
)
set_tests_properties(unit_sz_characterization PROPERTIES LABELS "unit")

add_test(
  NAME unit_diagonalcalc_characterization
  COMMAND sh ${CMAKE_SOURCE_DIR}/test/unit/diagonalcalc_characterization.sh
)
set_tests_properties(unit_diagonalcalc_characterization PROPERTIES LABELS "unit")

add_test(
  NAME unit_sz_calchs_validation
  COMMAND sh ${CMAKE_SOURCE_DIR}/test/unit/sz_calchs_validation.sh
)
set_tests_properties(unit_sz_calchs_validation PROPERTIES LABELS "unit")

add_test(
  NAME unit_sz_spin_calchs_minus1
  COMMAND sh ${CMAKE_SOURCE_DIR}/test/unit/sz_spin_calchs_minus1.sh
)
set_tests_properties(unit_sz_spin_calchs_minus1 PROPERTIES LABELS "unit")

add_test(
  NAME unit_sz_spin_calchs_zero
  COMMAND sh ${CMAKE_SOURCE_DIR}/test/unit/sz_spin_calchs_zero.sh
)
set_tests_properties(unit_sz_spin_calchs_zero PROPERTIES LABELS "unit")

add_test(
  NAME unit_sz_generalspin_characterization
  COMMAND sh ${CMAKE_SOURCE_DIR}/test/unit/sz_generalspin_characterization.sh
)
set_tests_properties(unit_sz_generalspin_characterization PROPERTIES LABELS "unit")

add_test(
  NAME unit_sz_spingc_characterization
  COMMAND sh ${CMAKE_SOURCE_DIR}/test/unit/sz_spingc_characterization.sh
)
set_tests_properties(unit_sz_spingc_characterization PROPERTIES LABELS "unit")

add_test(
  NAME unit_sz_kondogc_characterization
  COMMAND sh ${CMAKE_SOURCE_DIR}/test/unit/sz_kondogc_characterization.sh
)
set_tests_properties(unit_sz_kondogc_characterization PROPERTIES LABELS "unit")

add_test(
  NAME unit_sz_kondo_nconserved_characterization
  COMMAND sh ${CMAKE_SOURCE_DIR}/test/unit/sz_kondo_nconserved_characterization.sh
)
set_tests_properties(unit_sz_kondo_nconserved_characterization PROPERTIES LABELS "unit")

add_test(
  NAME unit_sz_tj_expert_characterization
  COMMAND sh ${CMAKE_SOURCE_DIR}/test/unit/sz_tj_expert_characterization.sh
)
set_tests_properties(unit_sz_tj_expert_characterization PROPERTIES LABELS "unit")

add_test(
  NAME unit_sz_tjgc_expert_characterization
  COMMAND sh ${CMAKE_SOURCE_DIR}/test/unit/sz_tjgc_expert_characterization.sh
)
set_tests_properties(unit_sz_tjgc_expert_characterization PROPERTIES LABELS "unit")

add_test(
  NAME unit_te_interall_diagonal_characterization
  COMMAND sh ${CMAKE_SOURCE_DIR}/test/unit/te_interall_diagonal_characterization.sh
)
set_tests_properties(unit_te_interall_diagonal_characterization PROPERTIES LABELS "unit")

add_test(
  NAME unit_diagonalcalc_kondo_characterization
  COMMAND sh ${CMAKE_SOURCE_DIR}/test/unit/diagonalcalc_kondo_characterization.sh
)
set_tests_properties(unit_diagonalcalc_kondo_characterization PROPERTIES LABELS "unit")

add_test(
  NAME unit_diagonalcalc_spingc_characterization
  COMMAND sh ${CMAKE_SOURCE_DIR}/test/unit/diagonalcalc_spingc_characterization.sh
)
set_tests_properties(unit_diagonalcalc_spingc_characterization PROPERTIES LABELS "unit")
