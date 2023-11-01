#!/bin/bash
#=
exec julia "${BASH_SOURCE[0]}" "$@"
=#

function make_ec_name(m, e, t, p, b, j, met, class_type)
  ec_name = "EC"
  if m > 0
    ec_name = string(ec_name, "_", m, "Muon")
  end
  if e > 0
    ec_name = string(ec_name, "_", e, "Electron")
  end
  if t > 0
    ec_name = string(ec_name, "_", t, "Tau")
  end
  if p > 0
    ec_name = string(ec_name, "_", p, "Photon")
  end
  if j > 0
    ec_name = string(ec_name, "_", j, "Jet")
  end
  if b > 0
    ec_name = string(ec_name, "_", b, "bJet")
  end
  if met > 0
    ec_name = string(ec_name, "_", met, "MET")
  end

  if class_type == "inclusive"
    ec_name = string(ec_name, "+X")
  end
  if class_type == "jet_inclusive"
    ec_name = string(ec_name, "+NJet")
  end

  return string("\"", ec_name, "\"")
end

global met = 1

function make_classes(filter_func::Function, class_type::String)
  for b = 0:9
    for j = 0:9
      for p = 0:9
        for t = 0:9
          for m = 0:9
            for e = 0:9
              if b + j <= 5
                if filter_func(m, e, t, p, b, j)
                  print(make_ec_name(m, e, t, p, b, j, met, class_type), ", ")
                end
              end
            end
          end
        end
      end
    end
  end
end

function one_muon(m::Int64, e::Int64, t::Int64, p::Int64, b::Int64, j::Int64)
  if m == 1 && e == 0 && t == 0 && p == 0
    return true
  end
  return false
end

function one_electron(m::Int64, e::Int64, t::Int64, p::Int64, b::Int64, j::Int64)
  if m == 0 && e == 1 && t == 0 && p == 0
    return true
  end
  return false
end
function two_muons(m::Int64, e::Int64, t::Int64, p::Int64, b::Int64, j::Int64)
  if m == 2 && e == 0 && t == 0 && p == 0
    return true
  end
  return false
end
function two_electrons(m::Int64, e::Int64, t::Int64, p::Int64, b::Int64, j::Int64)
  if m == 0 && e == 2 && t == 0 && p == 0
    return true
  end
  return false
end
function one_muon_one_electron(m::Int64, e::Int64, t::Int64, p::Int64, b::Int64, j::Int64)
  if m == 1 && e == 1 && t == 0 && p == 0
    return true
  end
  return false
end
function one_muon_or_one_electron_and_one_tau(m::Int64, e::Int64, t::Int64, p::Int64, b::Int64, j::Int64)
  if ((m == 1 && e == 0) || (m == 0 && e == 1)) && (j < 3) && t == 1 && p == 0
    return true
  end
  return false
end
function three_leptons_opossite_flavor(m::Int64, e::Int64, t::Int64, p::Int64, b::Int64, j::Int64)
  if (m + e + t == 3) && (b + j < 3) && p == 0
    return true
  end
  return false
end
function three_muons(m::Int64, e::Int64, t::Int64, p::Int64, b::Int64, j::Int64)
  if m == 3 && e == 0 && t == 0 && p == 0
    return true
  end
  return false
end
function three_electrons(m::Int64, e::Int64, t::Int64, p::Int64, b::Int64, j::Int64)
  if m == 0 && e == 3 && t == 0 && p == 0
    return true
  end
  return false
end
function multi_lepton_no_tau(m::Int64, e::Int64, t::Int64, p::Int64, b::Int64, j::Int64)
  if (m + e > 3) && m <= 4 && e <= 4 && t == 0 && p == 0
    return true
  end
  return false
end
function multi_lepton_with_tau(m::Int64, e::Int64, t::Int64, p::Int64, b::Int64, j::Int64)
  if (3 < m + e + t) && m <= 4 && e <= 4 && (m + e + t < 6) && p == 0
    return true
  end
  return false
end
function electron_photon(m::Int64, e::Int64, t::Int64, p::Int64, b::Int64, j::Int64)
  if m == 0 && e == 1 && t == 0 && p == 1
    return true
  end
  return false
end
function muon_photon(m::Int64, e::Int64, t::Int64, p::Int64, b::Int64, j::Int64)
  if m == 1 && e == 0 && t == 0 && p == 1
    return true
  end
  return false
end
function two_electron_photon(m::Int64, e::Int64, t::Int64, p::Int64, b::Int64, j::Int64)
  if m == 0 && e == 2 && t == 0 && p == 1
    return true
  end
  return false
end
function two_muon_photon(m::Int64, e::Int64, t::Int64, p::Int64, b::Int64, j::Int64)
  if m == 2 && e == 0 && t == 0 && p == 1
    return true
  end
  return false
end
function multi_tau_photon(m::Int64, e::Int64, t::Int64, p::Int64, b::Int64, j::Int64)
  if m == 0 && e == 0 && 0 < t && t <= 2 && p == 1
    return true
  end
  return false
end


######################
# main
function main()
  if first(size(ARGS)) > 0
    if ARGS[1] == "--help"
      println("This script will print the list of relevent classes, categorized.")
      exit(0)
    end

  end


  #
  # for filter = [one_muon, one_electron,two_muons, two_electrons, one_muon_one_electron, one_muon_or_one_electron_and_one_tau, three_leptons_opossite_flavor, three_muons, three_electrons, multi_lepton_no_tau, multi_lepton_with_tau,
  #   electron_photon, muon_photon, two_electron_photon, two_muon_photon, multi_tau_photon]
  #
  #   filter_name = String(Symbol(filter))
  #   println("## --> Classes: ", filter_name, "\n")
  #
  #   print(filter_name, "_exclusive = [")
  #   make_classes(filter, "exclusive")
  #   print("]\n\n\n")
  #
  #   print(filter_name, "_inclusive = [")
  #   make_classes(filter, "inclusive")
  #   print("]\n\n\n")
  #
  #   print(filter_name, "_jet_inclusive = [")
  #   make_classes(filter, "jet_inclusive")
  #   print("]\n\n\n")
  # end

  for filter = [one_muon, one_electron, two_muons, two_electrons, one_muon_one_electron, one_muon_or_one_electron_and_one_tau, three_leptons_opossite_flavor, three_muons, three_electrons, multi_lepton_no_tau, multi_lepton_with_tau,
    electron_photon, muon_photon, two_electron_photon, two_muon_photon, multi_tau_photon]

    filter_name = String(Symbol(filter))
    println("## --> Classes: ", filter_name, "\n")

    print(filter_name, "_exclusive_with_MET = [")
    make_classes(filter, "exclusive")
    print("]\n\n\n")

    print(filter_name, "_inclusive_with_MET = [")
    make_classes(filter, "inclusive")
    print("]\n\n\n")

    print(filter_name, "_jet_inclusive_with_MET = [")
    make_classes(filter, "jet_inclusive")
    print("]\n\n\n")
  end
end

main()


