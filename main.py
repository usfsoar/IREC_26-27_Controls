import rocketpy

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


# ENVIRONMENT
#TODO - environment analysis
#date- june 17, 2027 at 12pm
#set to irec 26 launch location for now
env = rocketpy.Environment(date=(2027, 6, 17, 12), latitude=0, longitude=0, elevation=0)
env.set_atmospheric_model(type="standard_atmosphere") #switch to a different model, maybe ensemble
# SOLID MOTOR CLASS

dry_mass = 3.269
length = 0.732
radius = 0.098 /2

i11 = (1/12) * dry_mass * (3*(radius**2) + length**2)
i22 =i11
i33 = .5*dry_mass*radius**2

M_motor = rocketpy.SolidMotor(
  thrust_source="AeroTech_M1939W.eng",
  dry_mass= dry_mass,
  dry_inertia=(i11, i22, i33),
  nozzle_radius= (1.75/2) / 39.37,
  grain_number=4,
  grain_density=0.06576*27679.90471 ,
  grain_outer_radius=3.365/2/39.37,
  grain_initial_inner_radius=1.125/2/39.37,
  grain_initial_height=6/2/39.37,
  grain_separation= 0,

  #these two are default, i have no idea
  grains_center_of_mass_position=0.385,
  center_of_dry_mass_position=0.366,

  nozzle_position=0,
  burn_time=6.2,
  throat_radius=.734/2/39.37,
  coordinate_system_orientation="nozzle_to_combustion_chamber",
)

#ROCKET BUILD

#mass without motor
nika = rocketpy.Rocket(
  radius=6, 
  mass=10, 
  inertia=(0,0,0), 
  power_off_drag=0, 
  power_on_drag=0,
  center_of_mass_without_motor=0,
  coordinate_system_orientation="tail_to_nose",
  )

nika.add_motor(M_motor, position=0)

nosecone = nika.add_nose(length=.5, kind="ogive", position=1.5)

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

#ADD Parachutes
main = nika.add_parachute(
    name="Main",
    cd_s=10.0,
    trigger=800,
    sampling_rate=105,
    lag=1.5,
    noise=(0, 8.3, 0.5),
    radius=1.5,
    height=1.5,
    porosity=0.0432,
)

drogue = nika.add_parachute(
    name="Drogue",
    cd_s=1.0,
    trigger="apogee",
    sampling_rate=105,
    lag=1.5,
    noise=(0, 8.3, 0.5),
    radius=1.5,
    height=1.5,
    porosity=0.0432,
)

nika.set_rail_buttons(
    upper_button_position=0.0818,
    lower_button_position=-0.618,
    angular_position=45,
)

flight = rocketpy.Flight(rocket=nika, environment=env, rail_length=10, inclination=6, heading=0)

# motor.all_info()
# env.prints.launch_site_details()