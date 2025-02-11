from Configs import GlobalParamsBuilder, JetParamsBuilder, Optional

if __name__ == "__main__":
    # jcf = JetParamsBuilder("Jet Params").set_jet_config_a(123.0).build()

    gcp = (
        GlobalParamsBuilder("Classification Test")
        .set_debug(False)
        .set_do_btag_efficiency(False)
        .set_filter_eff(1.0)
        .set_k_factor(1.0)
        .set_first_event(Optional.Null(int))
        .set_last_event(Optional.Null(int))
        .set_luminosity(333.0)
        .set_x_section(333.0)
        .set_generator_filter("")
        .set_input_files(["123", "abc"])
        .set_is_data(False)
        .set_is_dev_job(False)
        .set_output_file("output.root")
        .set_process("foo")
        .set_process_group("bar")
        .set_xs_order("NLO")
        .set_year("2025")
        .set_sum_weights_json_filepath("sum_weights_json_filepath")
        .build()
    )
