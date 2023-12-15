

// AUTOGENERATED FILE
#pragma once

#include "barretenberg/common/throw_or_abort.hpp"
#include "barretenberg/ecc/curves/bn254/fr.hpp"
#include "barretenberg/proof_system/circuit_builder/circuit_builder_base.hpp"

#include "barretenberg/flavor/generated/AvmMini_flavor.hpp"
#include "barretenberg/relations/generated/AvmMini/avm_mini.hpp"
#include "barretenberg/relations/generated/AvmMini/mem_trace.hpp"

using namespace barretenberg;

namespace proof_system {

template <typename FF> struct AvmMiniFullRow {
    FF avmMini_clk{};
    FF avmMini_first{};
    FF memTrace_m_clk{};
    FF memTrace_m_sub_clk{};
    FF memTrace_m_addr{};
    FF memTrace_m_val{};
    FF memTrace_m_lastAccess{};
    FF memTrace_m_rw{};
    FF avmMini_subop{};
    FF avmMini_ia{};
    FF avmMini_ib{};
    FF avmMini_ic{};
    FF avmMini_mem_op_a{};
    FF avmMini_mem_op_b{};
    FF avmMini_mem_op_c{};
    FF avmMini_rwa{};
    FF avmMini_rwb{};
    FF avmMini_rwc{};
    FF avmMini_mem_idx_a{};
    FF avmMini_mem_idx_b{};
    FF avmMini_mem_idx_c{};
    FF avmMini_last{};
    FF memTrace_m_addr_shift{};
    FF memTrace_m_rw_shift{};
    FF memTrace_m_val_shift{};
};

class AvmMiniCircuitBuilder {
  public:
    using Flavor = proof_system::honk::flavor::AvmMiniFlavor;
    using FF = Flavor::FF;
    using Row = AvmMiniFullRow<FF>;

    // TODO: template
    using Polynomial = Flavor::Polynomial;
    using ProverPolynomials = Flavor::ProverPolynomials;

    static constexpr size_t num_fixed_columns = 25;
    static constexpr size_t num_polys = 22;
    std::vector<Row> rows;

    void set_trace(std::vector<Row>&& trace) { rows = std::move(trace); }

    ProverPolynomials compute_polynomials()
    {
        const auto num_rows = get_circuit_subgroup_size();
        ProverPolynomials polys;

        // Allocate mem for each column
        for (auto& poly : polys.get_all()) {
            poly = Polynomial(num_rows);
        }

        for (size_t i = 0; i < rows.size(); i++) {
            polys.avmMini_clk[i] = rows[i].avmMini_clk;
            polys.avmMini_first[i] = rows[i].avmMini_first;
            polys.memTrace_m_clk[i] = rows[i].memTrace_m_clk;
            polys.memTrace_m_sub_clk[i] = rows[i].memTrace_m_sub_clk;
            polys.memTrace_m_addr[i] = rows[i].memTrace_m_addr;
            polys.memTrace_m_val[i] = rows[i].memTrace_m_val;
            polys.memTrace_m_lastAccess[i] = rows[i].memTrace_m_lastAccess;
            polys.memTrace_m_rw[i] = rows[i].memTrace_m_rw;
            polys.avmMini_subop[i] = rows[i].avmMini_subop;
            polys.avmMini_ia[i] = rows[i].avmMini_ia;
            polys.avmMini_ib[i] = rows[i].avmMini_ib;
            polys.avmMini_ic[i] = rows[i].avmMini_ic;
            polys.avmMini_mem_op_a[i] = rows[i].avmMini_mem_op_a;
            polys.avmMini_mem_op_b[i] = rows[i].avmMini_mem_op_b;
            polys.avmMini_mem_op_c[i] = rows[i].avmMini_mem_op_c;
            polys.avmMini_rwa[i] = rows[i].avmMini_rwa;
            polys.avmMini_rwb[i] = rows[i].avmMini_rwb;
            polys.avmMini_rwc[i] = rows[i].avmMini_rwc;
            polys.avmMini_mem_idx_a[i] = rows[i].avmMini_mem_idx_a;
            polys.avmMini_mem_idx_b[i] = rows[i].avmMini_mem_idx_b;
            polys.avmMini_mem_idx_c[i] = rows[i].avmMini_mem_idx_c;
            polys.avmMini_last[i] = rows[i].avmMini_last;
        }

        polys.memTrace_m_addr_shift = Polynomial(polys.memTrace_m_addr.shifted());
        polys.memTrace_m_rw_shift = Polynomial(polys.memTrace_m_rw.shifted());
        polys.memTrace_m_val_shift = Polynomial(polys.memTrace_m_val.shifted());

        return polys;
    }

    [[maybe_unused]] bool check_circuit()
    {
        ProverPolynomials polys = compute_polynomials();
        const size_t num_rows = polys.get_polynomial_size();

        const auto evaluate_relation = [&]<typename Relation>(const std::string& relation_name) {
            typename Relation::SumcheckArrayOfValuesOverSubrelations result;
            for (auto& r : result) {
                r = 0;
            }
            constexpr size_t NUM_SUBRELATIONS = result.size();

            for (size_t i = 0; i < num_rows; ++i) {
                Relation::accumulate(result, polys.get_row(i), {}, 1);

                bool x = true;
                for (size_t j = 0; j < NUM_SUBRELATIONS; ++j) {
                    if (result[j] != 0) {
                        throw_or_abort(
                            format("Relation ", relation_name, ", subrelation index ", j, " failed at row ", i));
                        x = false;
                    }
                }
                if (!x) {
                    return false;
                }
            }
            return true;
        };

        if (!evaluate_relation.template operator()<AvmMini_vm::mem_trace<FF>>("mem_trace")) {
            return false;
        }
        if (!evaluate_relation.template operator()<AvmMini_vm::avm_mini<FF>>("avm_mini")) {
            return false;
        }

        return true;
    }

    [[nodiscard]] size_t get_num_gates() const { return rows.size(); }

    [[nodiscard]] size_t get_circuit_subgroup_size() const
    {
        const size_t num_rows = get_num_gates();
        const auto num_rows_log2 = static_cast<size_t>(numeric::get_msb64(num_rows));
        size_t num_rows_pow2 = 1UL << (num_rows_log2 + (1UL << num_rows_log2 == num_rows ? 0 : 1));
        return num_rows_pow2;
    }
};
} // namespace proof_system
