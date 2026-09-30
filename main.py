import rocketpy

# https://docs.rocketpy.org/en/latest/user/motors/solidmotor.html
# https://www.rocketmotorparts.com/9810240_EMK__High_Power_Experimental_Motor_Kit/p1577809_14937531.aspx
# https://www.rocketmotorparts.com/product/98mm-nozzle-0-734%22-throat
# https://www.rocketmotorparts.com/98mm_White_Lightning__Propellant_Grain/p1577809_18244404.aspx
# https://www.rocketmotorparts.com/-Phenolic_Motor_Liner_RMS-9810240/p1577809_14676613.aspx
# https://www.thrustcurve.org/motors/AeroTech/M1939W/
# https://aerotech-rocketry.com/products/product_db085dc4-524f-0842-2c6f-418dc3e8a671
# https://wildmanrocketry.com/collections/98mm/products/9810240m




dry_mass = 3.269
length = 0.732
radius = 0.098 /2

i11 = (1/12) * dry_mass * (3*(radius**2) + length**2)
i22 =i11
i33 = .5*dry_mass*radius**2

motor = rocketpy.SolidMotor(
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

  grains_center_of_mass_position=0.385,
  center_of_dry_mass_position=0.366,
  nozzle_position=0,

  burn_time=6.2,
  throat_radius=.734/2/39.37,
  coordinate_system_orientation="nozzle_to_combustion_chamber",
)

print(motor.all_info())
