from csv import excel

import rocketpy
import pandas as pd


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
excel_environment = pd.read_excel('rpy_input_parameters.xlsx', sheet_name='Environment')
environment_params = dict(zip(excel_environment['Variable'], excel_environment['Set']))


#---INPUT-Solid Motor---
excel_solid_motor = pd.read_excel('rpy_input_parameters.xlsx', sheet_name='Solid Motor')
solid_motor_params = dict(zip(excel_solid_motor['Variable'], excel_solid_motor['Set']))

#---INPUT-Rocket---
excel_rocket = pd.read_excel('rpy_input_parameters.xlsx', sheet_name='Rocket')

# Dictionaries to separate fixed parameters from sweep/range parameters
params = {}
sweep_params = {}

for _, row in excel_rocket.iterrows():
    var_name = row['Variable']
    set_val = row['Set']
    
    # Check if 'Set' is empty (NaN or blank)
    if pd.isna(set_val) or str(set_val).strip() == '':
        # If set is empty, store the min, max, and step values
        sweep_params[f"min_{var_name}"] = row.get('min')
        sweep_params[f"max_{var_name}"] = row.get('max')
        sweep_params[f"step_{var_name}"] = row.get('step')
    else:
        # If set has a value, store it as a fixed parameter
        params[var_name] = set_val




#-----ENVIRONMENT-----
#TODO - environment analysis
#date- june 17, 2027 at 12pm
#set to irec 26 launch location for now
env = rocketpy.Environment(date=(environment_params['year'], environment_params['month'], environment_params['day'], environment_params['hour']), latitude=environment_params['latitude'], longitude=environment_params['longitude'], elevation=environment_params['elevation'])
env.set_atmospheric_model(type=environment_params['atmospheric_model']) #switch to a different model, maybe ensemble


#-----SOLID MOTOR CLASS-----
M_motor = rocketpy.SolidMotor(
    thrust_source=solid_motor_params['thrust_source'],
    dry_mass=float(solid_motor_params['dry_mass']),
    dry_inertia=(
        float(solid_motor_params['dry_inertia_i11']), 
        float(solid_motor_params['dry_inertia_i22']), 
        float(solid_motor_params['dry_inertia_i33']) ),
    nozzle_radius=solid_motor_params['nozzle_radius'],
    grain_number=solid_motor_params['grain_number'],
    grain_density=solid_motor_params['grain_density'],
    grain_outer_radius=solid_motor_params['grain_outer_radius'],
    grain_initial_inner_radius=solid_motor_params['grain_initial_inner_radius'],
    grain_initial_height=solid_motor_params['grain_initial_height'],
    grain_separation=solid_motor_params['grain_separation'],
    grains_center_of_mass_position=solid_motor_params['grains_center_of_mass_position'],
    center_of_dry_mass_position=solid_motor_params['center_of_dry_mass_position'],
    nozzle_position=solid_motor_params['nozzle_position'],
    burn_time=solid_motor_params['burn_time'],
    throat_radius=solid_motor_params['throat_radius'],
    coordinate_system_orientation=solid_motor_params['coordinate_system_orientation'],
)

# # #ROCKET BUILD

# #mass without motor

for mass in range(10, 21,2):
  nika = rocketpy.Rocket(
    radius=0.078359, 
    mass=mass, 
    inertia=(0,0,0), 
    power_off_drag=0, 
    power_on_drag=0,
    center_of_mass_without_motor=0,
    coordinate_system_orientation="tail_to_nose",
  ) 

  nika.add_motor(M_motor, position=0)

  for nc_length in range(1, 6):
    nosecone = nika.add_nose(
      length=.5, 
      kind="ogive", 
      position=1.5)

    nika.add_trapezoidal_fins(
      n=4,
      root_chord=0.120,
      tip_chord=0.060,
      span=0.110,
      position=-1.04956,
      cant_angle=0.5,
    )

    nika.add_tail(
        top_radius=0.0635, bottom_radius=0.0435, length=0.060, position=-1.194656
    )

    # #ADD Parachutes
    # main = nika.add_parachute(
    #     name="Main",
    #     cd_s=10.0,
    #     trigger=1000,
    #     sampling_rate=105,
    #     lag=1.5,
    #     noise=(0, 8.3, 0.5),
    #     radius=1.5,
    #     height=1.5,
    #     porosity=0.0432,
    # )

    # drogue = nika.add_parachute(
    #     name="Drogue",
    #     cd_s=1.0,
    #     trigger="apogee",
    #     sampling_rate=105,
    #     lag=1.5,
    #     noise=(0, 8.3, 0.5),
    #     radius=1.5,
    #     height=1.5,
    #     porosity=0.0432,
    # )

    # nika.set_rail_buttons(
    #     upper_button_position=0.0818,
    #     lower_button_position=-0.618,
    #     angular_position=45,
    # )

    flight = rocketpy.Flight(rocket=nika, environment=env, rail_length=10, inclination=6, heading=0)
    flight.plots.linear_kinematics_data()


# M_motor.all_info()
# env.prints.launch_site_details()