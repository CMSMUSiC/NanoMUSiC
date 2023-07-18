//////////////////////////
        // experimental new MET calculation for testing
        auto met_p4 = RVec<Math::PtEtaPhiMVector>{};
        auto scale_factors = RVec<double>{};
        auto scale_factor_up = RVec<double>{};
        auto scale_factor_down = RVec<double>{};
        auto delta_met_x = RVec<double>{};
        auto delta_met_y = RVec<double>{};
        auto is_fake = RVec<bool>{};
        // calculate met
        std::set<MUSiCObjects *> music_objects = {&muons, &electrons, &photons, &jets, &bjets};
        auto temp_met = Math::PtEtaPhiMVector(0, 0, 0, 0);
        for (auto &obj : music_objects)
        {
            for (size_t i = 0; i < obj->size(); i++)
            {
                temp_met -= obj->p4.at(i);
            }
        }
        // check for good met
        bool is_good_met = temp_met.pt() >= 100;
        if (is_good_met)
        {
            scale_factors.push_back(1.);
            scale_factor_up.push_back(1.);
            scale_factor_down.push_back(1.);
            met_p4.push_back(temp_met);
            delta_met_x.push_back(0.);
            delta_met_y.push_back(0.);
            is_fake.push_back(false);
        }
        met = MUSiCObjects(met_p4,            //
                           scale_factors,     //
                           scale_factor_up,   //
                           scale_factor_down, //
                           delta_met_x,       //
                           delta_met_y,       //
                           is_fake);
        //////////////////////////