#ifndef __INITIAL_H__
#define __INITIAL_H__

/**
 *   initial transponders, just add one or two entries for your network at
 *   the end of list...
 */
static
struct transponder qpsk_probes [] = {
	/**
	 *  Astra SDT info service
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 12551500,
			.inversion = INVERSION_AUTO,
			{ qpsk:
				{
					.symbol_rate = 22000000,
					.fec_inner = FEC_5_6
				}
			}
		},
		.polarisation = POLARISATION_VERTICAL
	},
        /**
         *  EUTELSAT SkyPlex, Hotbird 13E
         */
        {
                .type = FE_QPSK,
                .param = {
                        .frequency = 12539000,
                        .inversion = INVERSION_AUTO,
                        { qpsk:
                                {
                                        .symbol_rate = 27500000,
                                        .fec_inner = FEC_3_4
                                }
                        }
                },
                .polarisation = POLARISATION_HORIZONTAL
        },
	/**
	 *  Sirius Cinema 1
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 12054000,
			.inversion = INVERSION_AUTO,
			{ qpsk:
				{
					.symbol_rate = 27500000,
					.fec_inner = FEC_3_4
				}
			}
		},
		.polarisation = POLARISATION_VERTICAL
	},
	/**
	 *  Sirius S5.0E
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 11977000,
			.inversion = INVERSION_AUTO,
			{ qpsk: {
					.symbol_rate = 27500000,
					.fec_inner = FEC_3_4
				}
			}
		},
		.polarisation = POLARISATION_VERTICAL
	},
	/**
	 *  Sirius
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 11823000,
			.inversion = INVERSION_AUTO,
			{ qpsk:
				{
					.symbol_rate = 27500000,
					.fec_inner = FEC_3_4
				}
			}
		},
		.polarisation = POLARISATION_VERTICAL
	},
	/**
	 *  Sirius S5.0E Viasat
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 12054000,
			.inversion = INVERSION_AUTO,
			{ qpsk:
				{
					.symbol_rate = 27500000,
					.fec_inner = FEC_3_4
				}
			}
		},
		.polarisation = POLARISATION_VERTICAL
	},
	/**
	 *  Thor S1.0W
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 12226000,
			.inversion = INVERSION_AUTO,
			{ qpsk:
				{
					.symbol_rate = 28000000,
					.fec_inner = FEC_7_8
				}
			}
		},
		.polarisation = POLARISATION_VERTICAL
	},
	/**
	 *  Thor Telenor
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 11247000,
			.inversion = INVERSION_AUTO,
			{ qpsk:
				{
					.symbol_rate = 24500000,
					.fec_inner = FEC_7_8
				}
			}
		},
		.polarisation = POLARISATION_VERTICAL
	},
	/**
	 *  Thor Danish mux
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 11293000,
			.inversion = INVERSION_AUTO,
			{ qpsk:
				{
					.symbol_rate = 24500000,
					.fec_inner = FEC_7_8
				}
			}
		},
		.polarisation = POLARISATION_HORIZONTAL
	},
	/**
	 *  Thor Finnish Mux
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 12054000,
			.inversion = INVERSION_AUTO,
			{ qpsk:
				{
					.symbol_rate = 2800000,
					.fec_inner = FEC_7_8
				}
			}
		},
		.polarisation = POLARISATION_HORIZONTAL
	},
	/**
	 *  Canal Digital Thor S1.0W
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 11325000,
			.inversion = INVERSION_AUTO,
			{ qpsk:
				{
					.symbol_rate = 24500000,
					.fec_inner = FEC_7_8
				}
			}
		},
		.polarisation = POLARISATION_HORIZONTAL
	},
	/**
	 *  Thor 1.0W Bulgarian mux
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 12169000,
			.inversion = INVERSION_AUTO,
			{ qpsk:
				{
					.symbol_rate = 28000000,
					.fec_inner = FEC_7_8
				}
			}
		},
		.polarisation = POLARISATION_HORIZONTAL
	}, 
	/**
	 *  HISPASAT 1A/B, 30.0W vertical (TV de Galicia)
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 12015000,
			.inversion = INVERSION_AUTO,
			{ qpsk:
				{
					.symbol_rate = 27500000,
					.fec_inner = FEC_3_4
				}
			}
		},
		.polarisation = POLARISATION_VERTICAL
	},
	/**
	 *  HISPASAT 1C, 30.0W horizontal (Playboy TV)
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 11931000,
			.inversion = INVERSION_AUTO,
			{ qpsk:
				{
					.symbol_rate = 27500000,
					.fec_inner = FEC_3_4
				}
			}
		},
		.polarisation = POLARISATION_HORIZONTAL
	},
	/**
	 *  HISPASAT 1C, 30.0W vertical (Sol Musica Portugal)
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 11539000,
			.inversion = INVERSION_AUTO,
			{ qpsk:
				{
					.symbol_rate = 24500000,
					.fec_inner = FEC_5_6
				}
			}
		},
		.polarisation = POLARISATION_VERTICAL
	},
	/**
	 *  PAS 3 R, 30.0W horizontal (Fox Sports)
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 12567000, 
			.inversion = INVERSION_AUTO,
			{ qpsk:
				{
					.symbol_rate = 19850000,
					.fec_inner = FEC_3_4
				}
			}
		},
		.polarisation = POLARISATION_HORIZONTAL
	},
	/**
	 *  TELSTAR 12, 15.0W horizontal (Kuwait Space Channel)
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 12041000,
			.inversion = INVERSION_AUTO,
			{ qpsk:
				{
					.symbol_rate = 3256000,
					.fec_inner = FEC_2_3
				}
			}
		},
		.polarisation = POLARISATION_HORIZONTAL
	},
	/**
	 *  TELSTAR 12, 15.0W vertical (GlobeCast)
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 12520000,
			.inversion = INVERSION_AUTO,
			{ qpsk:
				{
					.symbol_rate = 8700000,
					.fec_inner = FEC_1_2
				}
			}
		},
		.polarisation = POLARISATION_VERTICAL
	},
	/**
	 *  EUTELSAT Telecom 2A/2D, 8.0W horizontal (Eurosport)
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 11635000,
			.inversion = INVERSION_AUTO,
			{ qpsk:
				{
					.symbol_rate = 6800000,
					.fec_inner = FEC_5_6
				}
			}
		},
		.polarisation = POLARISATION_HORIZONTAL
	},
	/**
	 *  EUTELSAT Telecom 2A/2D, 8.0W vertical (Eurosport)
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 12687000,
			.inversion = INVERSION_AUTO,
			{ qpsk:
				{
					.symbol_rate = 1879000,
					.fec_inner = FEC_3_4
				}
			}
		},
		.polarisation = POLARISATION_VERTICAL
	},
	/**
	 *  Turksat  1C, 42.0E horizontal (Galaxy TV)
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 11594000,
			.inversion = INVERSION_AUTO,
			{ qpsk:
				{
					.symbol_rate = 4557000,
					.fec_inner = FEC_5_6
				}
			}
		},
		.polarisation = POLARISATION_HORIZONTAL
	},
	/**
	 *  Turksat  1C, 42.0E vertical (Radio Cool)
	 */
	{
		.type = FE_QPSK,
		.param = {
			.frequency = 10978000,
			.inversion = INVERSION_AUTO,
			{ qpsk: {
					.symbol_rate = 2344000,
					.fec_inner = FEC_3_4
				}
			}
		},
		.polarisation = POLARISATION_VERTICAL
	},
};


static
struct transponder qam_probes [] = {
	/**
	 *  Kabel Berlin (3Sat, DLR Berlin, ...)
	 */
	{
		.type = FE_QAM,
		.param = {
			.frequency = 394000000,
			.inversion = INVERSION_AUTO,
			{ qam:
				{
					.symbol_rate = 6900000,
					.fec_inner = FEC_NONE,
					.modulation = QAM_64
				}
			}
		}
	},
	/**
	 *  Kabel Berlin (MTV Base, ...)
	 */
	{
		.type = FE_QAM,
		.param = {
			.frequency = 113000000,
			.inversion = INVERSION_AUTO,
			{ qam:
				{
					.symbol_rate = 6900000,
					.fec_inner = FEC_NONE,
					.modulation = QAM_64
				}
			}
		}
	},
	/**
	 *  Kabel Vienna - UPC Digital (RTL2, Sat1, ...)
	 */
	{
		.type = FE_QAM,
		.param = {
			.frequency = 377750000,
			.inversion = INVERSION_AUTO,
			{ qam:
				{
					.symbol_rate = 6900000,
					.fec_inner = FEC_NONE,
					.modulation = QAM_256
				}
			}
		}
	},
	/**
	 *  Kabel Suisse - IM-sfi (SF Info, ...)
	 */
	{
		.type = FE_QAM,
		.param = {
			.frequency = 530000000,
			.inversion = INVERSION_AUTO,
			{ qam:
				{
					.symbol_rate = 6900000,
					.fec_inner = FEC_NONE,
					.modulation = QAM_64
				}
			}
		}
	},
 	/**
	 *  Kabel iesy (Das Erste, ...)
	 */
	{
		.type = FE_QAM,
		.param = {
			.frequency = 410000000,
			.inversion = INVERSION_OFF,
			{ qam:
				{
					.symbol_rate = 6900000,
					.fec_inner = FEC_NONE,
					.modulation = QAM_64
				}
			}
		}
	},
        /**
	 *  Kabel 3KTV Finland (MHP Testikanava, ...)
	 */
	{
		.type = FE_QAM,
		.param = {
			.frequency = 306000000,
			.inversion = INVERSION_AUTO,
			{ qam:
				{
					.symbol_rate = 6875000,
					.fec_inner = FEC_NONE,
					.modulation = QAM_64
				}
			}
		}
	}
};


static
struct transponder ofdm_probes [] = {
	/**
	 *  Berlin (ProSieben, ...)
	 */
	{
		.type = FE_OFDM,
		.param = {
			.frequency = 658000000,
			.inversion = INVERSION_AUTO,
			{ ofdm:
				{
					.bandwidth = BANDWIDTH_8_MHZ,
					.code_rate_HP = FEC_2_3,
					.code_rate_LP = FEC_NONE,
					.constellation = QAM_16,
					.transmission_mode = TRANSMISSION_MODE_8K,
					.guard_interval = GUARD_INTERVAL_1_8,
					.hierarchy_information = HIERARCHY_NONE
				}
			}
		}
	},
	/**
	 *  Berlin (RTL, ...)
	 */
	{
		.type = FE_OFDM,
		.param = {
			.frequency = 778000000,
			.inversion = INVERSION_AUTO,
			{ ofdm:
				{
					.bandwidth = BANDWIDTH_8_MHZ,
					.code_rate_HP = FEC_2_3,
					.code_rate_LP = FEC_NONE,
					.constellation = QAM_16,
					.transmission_mode = TRANSMISSION_MODE_8K,
					.guard_interval = GUARD_INTERVAL_1_8,
					.hierarchy_information = HIERARCHY_NONE
				}
			}
		}
	},
	/**
	 *  Espoo A-mux (YLE channels, Provider Digita Finland)
	 */
	{
		.type = FE_OFDM,
		.param = {
			.frequency = 562000000,
			.inversion = INVERSION_OFF,
			{ ofdm:
				{
					.bandwidth = BANDWIDTH_8_MHZ,
					.code_rate_HP = FEC_2_3,
					.code_rate_LP = FEC_NONE,
					.constellation = QAM_64,
					.transmission_mode = TRANSMISSION_MODE_8K,
					.guard_interval = GUARD_INTERVAL_1_8,
					.hierarchy_information = HIERARCHY_NONE
				}
			}
		}
	},
	/**
	 *  Sandy Heath (BBC1, ...)
	 */
	{
		.type = FE_OFDM,
		.param = {
			.frequency = 641833334,
			.inversion = INVERSION_AUTO,
			{ ofdm:
				{
					.bandwidth = BANDWIDTH_8_MHZ,
					.code_rate_HP = FEC_2_3,
					.code_rate_LP = FEC_NONE,
					.constellation = QAM_64,
					.transmission_mode = TRANSMISSION_MODE_2K,
					.guard_interval = GUARD_INTERVAL_1_32,
					.hierarchy_information = HIERARCHY_NONE
				}
			}
		}
	},
	/**
	 *  Reigate (BBC1, ...)
	 */
	{
		.type = FE_OFDM,
		.param = {
			.frequency = 554000000,
			.inversion = INVERSION_AUTO,
			{ ofdm:
				{
					.bandwidth = BANDWIDTH_8_MHZ,
					.code_rate_HP = FEC_2_3,
					.code_rate_LP = FEC_NONE,
					.constellation = QAM_64,
					.transmission_mode = TRANSMISSION_MODE_2K,
					.guard_interval = GUARD_INTERVAL_1_32,
					.hierarchy_information = HIERARCHY_NONE
				}
			}
		}
	},
	/**
	 *  Oxford (BBC1, ...)
	 */
	{
		.type = FE_OFDM,
		.param = {
			.frequency = 578000000,
			.inversion = INVERSION_AUTO,
			{ ofdm:
				{
					.bandwidth = BANDWIDTH_8_MHZ,
					.code_rate_HP = FEC_2_3,
					.code_rate_LP = FEC_NONE,
					.constellation = QAM_64,
					.transmission_mode = TRANSMISSION_MODE_2K,
					.guard_interval = GUARD_INTERVAL_1_32,
					.hierarchy_information = HIERARCHY_NONE
				}
			}
		}
	},
	/**
	 *  Rowridge, Isle of Wight (BBC1, ...)
	 */
	{
		.type = FE_OFDM,
		.param = {
			.frequency = 489833333,
			.inversion = INVERSION_AUTO,
			{ ofdm:
				{
					.bandwidth = BANDWIDTH_8_MHZ,
					.code_rate_HP = FEC_3_4,
					.code_rate_LP = FEC_NONE,
					.constellation = QAM_16,
					.transmission_mode = TRANSMISSION_MODE_2K,
					.guard_interval = GUARD_INTERVAL_1_32,
					.hierarchy_information = HIERARCHY_NONE
				}
			}
		}
	},
	/**
	 *  Crystal Palace, (BBC1, ...)
	 */
	{
		.type = FE_OFDM,
		.param = {
			.frequency = 505833333,
			.inversion = INVERSION_AUTO,
			{ ofdm:
				{
					.bandwidth = BANDWIDTH_8_MHZ,
					.code_rate_HP = FEC_3_4,
					.code_rate_LP = FEC_NONE,
					.constellation = QAM_16,
					.transmission_mode = TRANSMISSION_MODE_2K,
					.guard_interval = GUARD_INTERVAL_1_32,
					.hierarchy_information = HIERARCHY_NONE
				}
			}
		}
	},
	/**
	 *  Hannington, North Hampshire, (BBC1, ...)
	 */
	{
		.type = FE_OFDM,
		.param = {
			.frequency = 706000000,
			.inversion = INVERSION_AUTO,
			{ ofdm:
				{
					.bandwidth = BANDWIDTH_8_MHZ,
					.code_rate_HP = FEC_3_4,
					.code_rate_LP = FEC_NONE,
					.constellation = QAM_16,
					.transmission_mode = TRANSMISSION_MODE_2K,
					.guard_interval = GUARD_INTERVAL_1_32,
					.hierarchy_information = HIERARCHY_NONE
				}
			}
		}
	},
};


#endif

