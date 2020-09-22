#include "escape_hatch.hpp"
#include "../join_split/join_split_circuit.hpp"
#include "../notes/note_pair.hpp"
#include "../rollup/rollup_circuit.hpp"

// #pragma GCC diagnostic ignored "-Wunused-variable"
// #pragma GCC diagnostic ignored "-Wunused-parameter"
namespace rollup {
namespace proofs {
namespace escape_hatch {

using namespace plonk::stdlib::types::turbo;
using namespace plonk::stdlib::merkle_tree;
using namespace join_split;

void escape_hatch_circuit(Composer& composer, escape_hatch_tx const& tx)
{
    join_split_inputs inputs = {
        witness_ct(&composer, tx.js_tx.public_input),
        witness_ct(&composer, tx.js_tx.public_output),
        witness_ct(&composer, tx.js_tx.num_input_notes),
        witness_ct(&composer, tx.js_tx.input_index[0]),
        witness_ct(&composer, tx.js_tx.input_index[1]),
        create_note_pair(composer, tx.js_tx.input_note[0]),
        create_note_pair(composer, tx.js_tx.input_note[1]),
        create_note_pair(composer, tx.js_tx.output_note[0]),
        create_note_pair(composer, tx.js_tx.output_note[1]),
        { witness_ct(&composer, tx.js_tx.signing_pub_key.x), witness_ct(&composer, tx.js_tx.signing_pub_key.y) },
        stdlib::schnorr::convert_signature(&composer, tx.js_tx.signature),
        witness_ct(&composer, tx.js_tx.old_data_root),
        merkle_tree::create_witness_hash_path(composer, tx.js_tx.input_path[0]),
        merkle_tree::create_witness_hash_path(composer, tx.js_tx.input_path[1]),
        witness_ct(&composer, tx.js_tx.account_index),
        merkle_tree::create_witness_hash_path(composer, tx.js_tx.account_path),
        witness_ct(&composer, tx.js_tx.output_owner),
    };

    auto outputs = join_split_circuit_component(composer, inputs);

    // TODO: should be able to be a constant, but causes things to fail :/ (does it still?)
    auto one = uint32_ct(witness_ct(&composer, 1));
    auto rollup_id = field_ct(witness_ct(&composer, tx.rollup_id));
    auto old_data_root = field_ct(witness_ct(&composer, tx.js_tx.old_data_root));
    auto new_data_root = field_ct(witness_ct(&composer, tx.new_data_root));
    auto old_data_roots_root = field_ct(witness_ct(&composer, tx.old_data_roots_root));
    auto new_data_roots_root = field_ct(witness_ct(&composer, tx.new_data_roots_root));
    auto old_null_root = field_ct(witness_ct(&composer, tx.old_null_root));
    auto data_start_index = field_ct(witness_ct(&composer, tx.data_start_index));

    auto new_null_root = rollup::check_nullifiers_inserted(composer,
                                                           tx.new_null_roots,
                                                           tx.old_null_paths,
                                                           tx.new_null_paths,
                                                           one,
                                                           old_null_root,
                                                           { outputs.nullifier1, outputs.nullifier2 });

    rollup::check_root_tree_updated(composer,
                                    create_witness_hash_path(composer, tx.new_data_roots_path),
                                    create_witness_hash_path(composer, tx.old_data_roots_path),
                                    rollup_id,
                                    new_data_root,
                                    new_data_roots_root,
                                    old_data_roots_root);

    rollup::check_data_tree_updated(composer,
                                    1,
                                    create_witness_hash_path(composer, tx.new_data_path),
                                    create_witness_hash_path(composer, tx.old_data_path),
                                    { byte_array_ct(&composer)
                                          .write(inputs.output_note1.second.ciphertext.x)
                                          .write(inputs.output_note1.second.ciphertext.y),
                                      byte_array_ct(&composer)
                                          .write(inputs.output_note2.second.ciphertext.x)
                                          .write(inputs.output_note2.second.ciphertext.y) },
                                    old_data_root,
                                    new_data_root,
                                    data_start_index);

    rollup::check_accounts_not_nullified(
        composer, one, old_null_root, { outputs.account_nullifier }, { tx.account_null_path });

    // Public inputs mimick a 1 rollup, minus the pairing point at the end.
    composer.set_public_input(rollup_id.witness_index);
    public_witness_ct(&composer, 0); // rollup_size. 0 implies escape hatch.
    composer.set_public_input(data_start_index.witness_index);
    composer.set_public_input(old_data_root.witness_index);
    composer.set_public_input(new_data_root.witness_index);
    composer.set_public_input(old_null_root.witness_index);
    composer.set_public_input(new_null_root.witness_index);
    composer.set_public_input(old_data_roots_root.witness_index);
    composer.set_public_input(new_data_roots_root.witness_index);
    public_witness_ct(&composer, 1); // num_txs.

    // "Inner proof".
    public_witness_ct(&composer, 0); // proof_id.
    composer.set_public_input(inputs.public_input.witness_index);
    composer.set_public_input(inputs.public_output.witness_index);
    composer.set_public_input(inputs.output_note1.second.ciphertext.x.witness_index);
    composer.set_public_input(inputs.output_note1.second.ciphertext.y.witness_index);
    composer.set_public_input(inputs.output_note2.second.ciphertext.x.witness_index);
    composer.set_public_input(inputs.output_note2.second.ciphertext.y.witness_index);
    composer.set_public_input(outputs.nullifier1.witness_index);
    composer.set_public_input(outputs.nullifier2.witness_index);
    public_witness_ct(&composer, tx.js_tx.input_owner);
    public_witness_ct(&composer, tx.js_tx.output_owner);
}

} // namespace escape_hatch
} // namespace proofs
} // namespace rollup