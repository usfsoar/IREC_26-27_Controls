import rocketpy
import pandas as pd
import numpy as np
from concurrent.futures import ProcessPoolExecutor
import csv

#multiprocess, early filtering (stability), post filter (apogee between 10k and 12k, stab margin between 1 and 1.75)
#rocketpy sims are not useful to make solution space. rpy monete carlo is meant to handle stochastic variables and see how a single design behaves
#custom sim gives total deterministic control, systematically go through each precise strucutureal changes. my script can enforce enigneering boundaries on the fly
#need to look into algorithms instead of brute force like genetic algorithms (deap, pygad), latin hypercube sampling, bayesian optimization

## Motor Sources
# https://docs.rocketpy.org/en/latest/user/motors/solidmotor.html
# https://www.rocketmotorparts.com/9810240_EMK__High_Power_Experimental_Motor_Kit/p1577809_14937531.aspx
# https://www.rocketmotorparts.com/product/98mm-nozzle-0-734%22-throat
# https://www.rocketmotorparts.com/98mm_White_Lightning__Propellant_Grain/p1577809_18244404.aspx
# https://www.rocketmotorparts.com/-Phenolic_Motor_Liner_RMS-9810240/p1577809_14676613.aspx
# https://www.thrustcurve.org/motors/AeroTech/M1939W/
# https://aerotech-rocketry.com/products/product_db085dc4-524f-0842-2c6f-418dc3e8a671
# https://wildmanrocketry.com/collections/98mm/products/9810240m

#---INPUT-Environment---
excel_environment = pd.read_excel("rpy_input_parameters.xlsx", sheet_name="Environment")
environment_params = dict(zip(excel_environment["Variable"], excel_environment["Set"]))


#---INPUT-Solid Motor---
excel_solid_motor = pd.read_excel("rpy_input_parameters.xlsx", sheet_name="Solid Motor")
solid_motor_params = dict(zip(excel_solid_motor["Variable"], excel_solid_motor["Set"]))

#---INPUT-Rocket---
excel_rocket = pd.read_excel("rpy_input_parameters.xlsx", sheet_name="Rocket")
rocket_params = {}
rocket_sweep_params = {}

for _, row in excel_rocket.iterrows():
    var_name = row["Variable"]
    set_val = row["Set"]
    
    if pd.isna(set_val) or str(set_val).strip() == "":
        rocket_sweep_params[f"min_{var_name}"] = float(row.get("Min"))
        rocket_sweep_params[f"max_{var_name}"] = float(row.get("Max"))
        rocket_sweep_params[f"step_{var_name}"] = float(row.get("Step"))
    else:
        if str(set_val).strip() == "None" or pd.isna(set_val) or str(set_val) == "":
            rocket_params[var_name] = None
        else:
            rocket_params[var_name] = set_val


#---INPUT-Nosecone---
excel_nosecone = pd.read_excel("rpy_input_parameters.xlsx", sheet_name="Nosecone")
nosecone_params = {}
nosecone_sweep_params = {}

for _, row in excel_nosecone.iterrows():
    var_name = row["Variable"]
    set_val = row["Set"]
    
    if pd.isna(set_val) or str(set_val).strip() == "":
        nosecone_sweep_params[f"min_{var_name}"] = float(row.get("Min"))
        nosecone_sweep_params[f"max_{var_name}"] = float(row.get("Max"))
        nosecone_sweep_params[f"step_{var_name}"] = float(row.get("Step"))
    else:
        if str(set_val).strip() == "none" or pd.isna(set_val) or str(set_val).strip() == "":
            nosecone_params[var_name] = None
        else:
            nosecone_params[var_name] = set_val


#---INPUT-Fins---
excel_fins = pd.read_excel("rpy_input_parameters.xlsx", sheet_name="Trapezoidal Fins")
fins_params = {}
fins_sweep_params = {}

for _, row in excel_fins.iterrows():
    var_name = row["Variable"]
    set_val = row["Set"]
    
    if pd.isna(set_val) or str(set_val).strip() == "":
        fins_sweep_params[f"min_{var_name}"] = float(row.get("Min"))
        fins_sweep_params[f"max_{var_name}"] = float(row.get("Max"))
        fins_sweep_params[f"step_{var_name}"] = float(row.get("Step"))
    else:
        if str(set_val).strip() == "none" or pd.isna(set_val) or str(set_val).strip() == "":
            fins_params[var_name] = None
        else:
            fins_params[var_name] = set_val



#---INPUT-Rail Buttons---
excel_buttons = pd.read_excel("rpy_input_parameters.xlsx", sheet_name="Rail Buttons")
buttons_params = {}
buttons_sweep_params = {}

for _, row in excel_buttons.iterrows():
    var_name = row["Variable"]
    set_val = row["Set"]
    
    if pd.isna(set_val) or str(set_val).strip() == "":
        buttons_sweep_params[f"min_{var_name}"] = float(row.get("Min"))
        buttons_sweep_params[f"max_{var_name}"] = float(row.get("Max"))
        buttons_sweep_params[f"step_{var_name}"] = float(row.get("Step"))
    else:
        if str(set_val) == "none" or pd.isna(set_val) or str(set_val) == "":
            buttons_params[var_name] = None
        else:
            buttons_params[var_name] = set_val


#---INPUT-Flight---
excel_buttons = pd.read_excel("rpy_input_parameters.xlsx", sheet_name="Flight")
flight_params = {}
flight_sweep_params = {}

for _, row in excel_buttons.iterrows():
    var_name = row["Variable"]
    set_val = row["Set"]
    
    if pd.isna(set_val) or str(set_val).strip() == "":
        flight_sweep_params[f"min_{var_name}"] = float(row.get("Min"))
        flight_sweep_params[f"max_{var_name}"] = float(row.get("Max"))
        flight_sweep_params[f"step_{var_name}"] = float(row.get("Step"))
    else:
        if str(set_val) == "none" or pd.isna(set_val) or str(set_val) == "":
            flight_params[var_name] = None
        elif str(set_val).strip().replace('.', '', 1).lstrip('-').isdigit():
            flight_params[var_name] = float(set_val)
        else:
            flight_params[var_name] = set_val


def append_to_file(data):
  with open("rpy_output.csv", "a", newline="", encoding="utf-8") as f:
    writer = csv.writer(f)
    writer.writerow(data)


def run_single_simulation(
    rocket_params,
    rocket_sweep_params,
    nosecone_params,
    nosecone_sweep_params,
    fins_params,
    fins_sweep_params,
    buttons_params,
    buttons_sweep_params,
    flight_params,
    flight_sweep_params,
    env,
    M_motor,
    ):

    nika = rocketpy.Rocket(
        radius=rocket_params["radius"], 
        mass=rocket_sweep_params["mass"], 
        inertia=(0,0,0), 
        power_off_drag=0, 
        power_on_drag=0,
        center_of_mass_without_motor=0,
        coordinate_system_orientation=rocket_params["coordinate_system_orientation"],
    ) 
    
    nika.add_motor(M_motor, position=0)
    
    nika.nosecone = nika.add_nose(
        length=nosecone_sweep_params["length"], 
        kind=nosecone_params["kind"], 
        position=nosecone_params["position"],
        bluffness=nosecone_sweep_params["bluffness"],
        power=nosecone_params["power"],
        name=nosecone_params["name"],
        base_radius=nosecone_params["base_radius"],
    )

# for fins_sweep_params
    nika.add_trapezoidal_fins(
        n=fins_params["n"],
        root_chord=fins_sweep_params["root_chord"],
        tip_chord=fins_sweep_params["tip_chord"],
        span=fins_sweep_params["span"],
        position=fins_sweep_params["position"],
        cant_angle=fins_sweep_params["cant_angle"],
        sweep_angle=fins_params["sweep_angle"],
        sweep_length=fins_sweep_params["sweep_length"],
        radius=fins_params["radius"],
        airfoil=fins_params["airfoil"],
    )

    nika.set_rail_buttons(
        upper_button_position=buttons_sweep_params["upper_button_position"],
        lower_button_position=buttons_sweep_params["lower_button_position"],
        angular_position=buttons_params["angular_position"],
        radius=buttons_params["radius"]
    )  


    flight = rocketpy.Flight(
        rocket=nika,
        environment=env,
        rail_length=flight_sweep_params["rail_length"], 
        inclination=flight_sweep_params["inclination"],
        heading=flight_params["heading"],
        initial_solution=flight_params["initial_solution"],
        terminate_on_apogee=bool(flight_params["terminate_on_apogee"]),
        max_time=flight_params["max_time"],
        max_time_step=flight_params["max_time_step"],
        min_time_step=flight_params["min_time_step"],
        #rtol=flight_params["rtol"],
        #atol=flight_params["atol"],
        time_overshoot=flight_params["time_overshoot"],
        verbose=bool(flight_params["verbose"]),
        equations_of_motion=flight_params["equations_of_motion"],
        simulation_mode=flight_params["simulation_mode"],
        )

    # flutter_mach, safety_factor = rocketpy.utilities.fin_flutter_analysis(
    #     fin_thickness=fins_params["fin_thickness"],
    #     shear_modulus=fins_params["shear_modulus"], 
    #     flight=flight,
    #     see_prints=True,
    #     see_graphs=True
    # )

    return [flight.apogee]#, flight.initial_stability_margin, flight.out_of_rail_state, flight.out_of_rail_stability_margin, flight.max_dynamic_pressure, safety_factor(flight.max_dynamic_pressure_time)]



#-----ENVIRONMENT-----
#TODO - environment and weather analysis
#date- june 17, 2027 at 12pm
#TODO - set to irec 26 launch location for now
env = rocketpy.Environment(date=(environment_params["year"], environment_params["month"], environment_params["day"], environment_params["hour"]), latitude=environment_params["latitude"], longitude=environment_params["longitude"], elevation=environment_params["elevation"])
env.set_atmospheric_model(type=environment_params["atmospheric_model"]) #switch to a different model, maybe ensemble


#-----SOLID MOTOR CLASS-----
M_motor = rocketpy.SolidMotor(
    thrust_source=solid_motor_params["thrust_source"],
    dry_mass=float(solid_motor_params["dry_mass"]),
    dry_inertia=(
        float(solid_motor_params["dry_inertia_i11"]), 
        float(solid_motor_params["dry_inertia_i22"]), 
        float(solid_motor_params["dry_inertia_i33"]) ),
    nozzle_radius=solid_motor_params["nozzle_radius"],
    grain_number=solid_motor_params["grain_number"],
    grain_density=solid_motor_params["grain_density"],
    grain_outer_radius=solid_motor_params["grain_outer_radius"],
    grain_initial_inner_radius=solid_motor_params["grain_initial_inner_radius"],
    grain_initial_height=solid_motor_params["grain_initial_height"],
    grain_separation=solid_motor_params["grain_separation"],
    grains_center_of_mass_position=solid_motor_params["grains_center_of_mass_position"],
    center_of_dry_mass_position=solid_motor_params["center_of_dry_mass_position"],
    nozzle_position=solid_motor_params["nozzle_position"],
    burn_time=solid_motor_params["burn_time"],
    throat_radius=solid_motor_params["throat_radius"],
    coordinate_system_orientation=solid_motor_params["coordinate_system_orientation"],
)



count = 0
with ProcessPoolExecutor(max_workers=4) as executor:
    sims = []
    for rocket_sweep_params["mass"] in np.arange(rocket_sweep_params["min_mass"], rocket_sweep_params["max_mass"] + rocket_sweep_params["step_mass"], rocket_sweep_params["step_mass"]):
        for nosecone_sweep_params["length"] in np.arange(nosecone_sweep_params["min_length"], nosecone_sweep_params["max_length"] + nosecone_sweep_params["step_length"], nosecone_sweep_params["step_length"]):
            for nosecone_sweep_params["bluffness"] in np.arange(nosecone_sweep_params["min_bluffness"], nosecone_sweep_params["max_bluffness"] + nosecone_sweep_params["step_bluffness"], nosecone_sweep_params["step_bluffness"]):
                for flight_sweep_params["rail_length"] in np.arange(flight_sweep_params["min_rail_length"], flight_sweep_params["max_rail_length"] + flight_sweep_params["step_rail_length"], flight_sweep_params["step_rail_length"]):
                    for flight_sweep_params["inclination"] in np.arange(flight_sweep_params["min_inclination"], flight_sweep_params["max_inclination"] + flight_sweep_params["step_inclination"], flight_sweep_params["step_inclination"]):
                        for fins_sweep_params["span"] in np.arange(fins_sweep_params["min_span"], fins_sweep_params["max_span"] + fins_sweep_params["step_span"], fins_sweep_params["step_span"]):
                            for fins_sweep_params["root_chord"] in np.arange(fins_sweep_params["min_root_chord"], fins_sweep_params["max_root_chord"] + fins_sweep_params["step_root_chord"], fins_sweep_params["step_root_chord"]):
                                for fins_sweep_params["tip_chord"] in np.arange(fins_sweep_params["min_tip_chord"], fins_sweep_params["max_tip_chord"] + fins_sweep_params["step_tip_chord"], fins_sweep_params["step_tip_chord"]):
                                    for fins_sweep_params["position"] in np.arange(fins_sweep_params["min_position"], fins_sweep_params["max_position"] + fins_sweep_params["step_position"], fins_sweep_params["step_position"]):
                                        for fins_sweep_params["cant_angle"] in np.arange(fins_sweep_params["min_cant_angle"], fins_sweep_params["max_cant_angle"] + fins_sweep_params["step_cant_angle"], fins_sweep_params["step_cant_angle"]):
                                            for fins_sweep_params["sweep_length"] in np.arange(fins_sweep_params["min_sweep_length"], fins_sweep_params["max_sweep_length"] + fins_sweep_params["step_sweep_length"], fins_sweep_params["step_sweep_length"]):
                                                for buttons_sweep_params["upper_button_position"] in np.arange(buttons_sweep_params["min_upper_button_position"], buttons_sweep_params["max_upper_button_position"] + buttons_sweep_params["step_upper_button_position"], buttons_sweep_params["step_upper_button_position"]):
                                                    
                                                    lower_button_position = np.arange(buttons_sweep_params["min_lower_button_position"], buttons_sweep_params["max_lower_button_position"] + buttons_sweep_params["step_lower_button_position"], buttons_sweep_params["step_lower_button_position"])
                                                    
                                                    for low_but_pos in lower_button_position:
                                                        buttons_sweep_params["lower_button_position"] = low_but_pos
                                                        run_single_simulation(
                                                            rocket_params,
                                                            rocket_sweep_params,
                                                            nosecone_params,
                                                            nosecone_sweep_params,
                                                            fins_params,
                                                            fins_sweep_params,
                                                            buttons_params,
                                                            buttons_sweep_params,
                                                            flight_params,
                                                            flight_sweep_params,
                                                            env,
                                                            M_motor
                                                        )
                                                        count += 1
                                                        print(count)
    for sim in sims:
        append_to_file(sim.result())
                                                    




# M_motor.all_info()
# env.prints.launch_site_details()