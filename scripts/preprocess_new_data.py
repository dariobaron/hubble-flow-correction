import pandas as pd
from matplotlib import pyplot as plt
import seaborn as sb
import numpy as np
import h5py
from tqdm import tqdm


def processColumns(df_gal, df_cl):
	df_gal = df_gal.copy()
	df_cl = df_cl.copy()
	# filtering out central galaxies
	mask = np.all(df_gal[["x","y","z"]] != 0, axis=1)
	df_gal = df_gal.loc[mask]
	# using cluster identifier as unsigned int
	df_gal["icl"] = df_gal["icl"].astype("u4")
	df_cl["icl"] = df_cl["icl"].astype("u4")
	df_cl.set_index("icl", inplace=True)
	# accounting for Uchuu universe
	h = 0.6774
	H0 = 100 * h # km/s/Mpc
	# adding Hubble flow to velocities
	df_gal[["vx","vy","vz"]] = df_gal[["vx","vy","vz"]] + H0 * df_gal[["x","y","z"]].to_numpy()
	# changing reference frame from comoving to physical
	df_gal[["x","y","z","Mvir","Mstr"]] = df_gal[["x","y","z","Mvir","Mstr"]] * h
	df_cl[["Rvcl","Mvcl"]] = df_cl[["Rvcl","Mvcl"]] * h
	# normalizing positions to virial cluster radius
	Rvcl = df_cl["Rvcl"].loc[df_gal["icl"]].to_numpy()
	df_gal[["x","y","z"]] = df_gal[["x","y","z"]].div(Rvcl, axis=0)
	# computing 3D and projected distances
	df_gal["R"] = np.linalg.norm(df_gal[["x","y","z"]], axis=1)
	df_gal["r"] = np.linalg.norm(df_gal[["x","y"]], axis=1)
	# normalizing velocities to cluster circular velocity
	G_univ = 4.3009172706e-9 # Mpc·(km/s)^2 / Msun
	df_cl["Vvcl"] = np.sqrt(G_univ * df_cl["Mvcl"] / df_cl["Rvcl"])
	Vvcl = df_cl["Vvcl"].loc[df_gal["icl"]].to_numpy()
	df_gal[["vx","vy","vz"]] = df_gal[["vx","vy","vz"]].div(Vvcl, axis=0)
	# computing radial and tangential velocities
	df_gal["vR"] = (df_gal["x"]*df_gal["vx"] + df_gal["y"]*df_gal["vy"] + df_gal["z"]*df_gal["vz"]) / df_gal["R"]
	df_gal["vtot"] = np.linalg.norm(df_gal[["vx","vy","vz"]], axis=1)
	df_gal["vTang"] = np.sqrt(df_gal["vtot"]**2 - df_gal["vR"]**2)
	# reordering columns
	galaxy_properties = ["x", "y", "z", "R", "r", "vx", "vy", "vz", "vR", "vTang", "vtot", "Mvir", "Mstr", "haloID", "upID", "icl"]
	df_gal = df_gal[galaxy_properties]
	cluster_properties = ["Mvcl", "Rvcl", "Vvcl", "Jx", "Jy", "Jz", "Lambda", "bovera", "covera", "Ax", "Ay", "Az", "scllastMM", "origclid"]
	df_cl = df_cl[cluster_properties]
	return df_gal, df_cl


def saveData(outfilename, data, columns, part, overwrite=False):
	df = pd.DataFrame(data, columns=columns)
	galaxy_properties = ["x", "y", "z", "vx", "vy", "vz", "Mvir", "Mstr", "haloID", "upID", "icl"]
	cluster_properties = ["icl", "origclid", "Mvcl", "Rvcl", "Jx", "Jy", "Jz", "Lambda", "bovera", "covera", "Ax", "Ay", "Az", "scllastMM"]
	# separating galaxy and cluster properties
	df_gal = df[galaxy_properties]
	df_cl = df[cluster_properties]
	df_cl = df_cl.drop_duplicates()
	# processing columns
	df_gal, df_cl = processColumns(df_gal, df_cl)
	# saving to hdf5
	records_gal = df_gal.to_records(index=False)
	records_cl = df_cl.to_records(index=True)
	with h5py.File(outfilename, "a") as outfile:
		group = outfile.require_group(f"{part:02d}")
		if overwrite and "galaxies" in group:
			del group["galaxies"]
		if "galaxies" not in group:
			group.create_dataset("galaxies", data=records_gal, compression="gzip", compression_opts=9)
		if overwrite and "clusters" in group:
			del group["clusters"]
		if "clusters" not in group:
			group.create_dataset("clusters", data=records_cl, compression="gzip", compression_opts=9)


def readDatFile(infilename, outfilename, batch_size, overwrite=False):
	data = []
	with open(infilename,"r") as inputfile:
		for i,line in tqdm(enumerate(inputfile)):
			if i == 0:
				columns = line.strip().split()
				columns.remove("#")
				continue
			values = line.strip().split()
			values = [float(v) for v in values]
			data.append(values)
			if not i % batch_size:
				saveData(outfilename, data, columns, i//batch_size, overwrite=overwrite)
				data = []
	if data:
		saveData(outfilename, data, columns, (i//batch_size)+1, overwrite=overwrite)
		data = []


def consolidateData(filename, overwrite=False):
	dfs_cl = []
	with h5py.File(filename) as file:
		for group in tqdm(file.values(), desc="Merging clusters", dynamic_ncols=True):
			if isinstance(group, h5py.Group):
				df_cl = pd.DataFrame.from_records(group["clusters"][:])
				dfs_cl.append(df_cl)
	df_all_cl = pd.concat(dfs_cl, ignore_index=True)
	df_all_cl.drop_duplicates(inplace=True)
	records_all_cl = df_all_cl.to_records(index=False)
	with h5py.File(filename, "a") as file:
		if overwrite and "clusters" in file:
			del file["clusters"]
		if "clusters" not in file:
			file.create_dataset("clusters", data=records_all_cl, compression="gzip", compression_opts=9)
	del dfs_cl, df_cl, df_all_cl, records_all_cl
	dfs_gal = []
	with h5py.File(filename) as file:
		for group in tqdm(file.values(), desc="Merging galaxies", dynamic_ncols=True):
			if isinstance(group, h5py.Group):
				df_gal = pd.DataFrame.from_records(group["galaxies"][:])
				dfs_gal.append(df_gal)
	df_all_gal = pd.concat(dfs_gal, ignore_index=True)
	records_all_gal = df_all_gal.to_records(index=False)
	with h5py.File(filename, "a") as file:
		if overwrite and "galaxies" in file:
			del file["galaxies"]
		if "galaxies" not in file:
			file.create_dataset("galaxies", data=records_all_gal, compression="gzip", compression_opts=9)
	del dfs_gal, df_gal, df_all_gal, records_all_gal


if __name__ == "__main__":
	datafolder = "data/"
	basename = "UchuuLong"
	infilename = datafolder + basename + ".dat"
	outfilename = datafolder + basename + ".h5"
	batch_size = 500000
	readDatFile(infilename, outfilename, batch_size, overwrite=True)
	consolidateData(outfilename, overwrite=True)