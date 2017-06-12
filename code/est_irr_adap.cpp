vecf4 est_irr_adap(const vecf4& pos, const vecf4& n, uint& sample_count){
	const uint max_count = max_sample_count;
	const uint min_count = 256;
	const float rel_err_eps = 0.0001f;

	// initialisiere temporaere Variablen
	vecf4 sum(0,0,0);
	vecf4 sum_sq(0,0,0);
	uint count = 0;
	float rel_err = 0.0f;

	// generiere weitere Samples, bis Abbruchbedingung erreicht
	while ((count < max_count) && ((count < min_count) || (rel_err > rel_err_bound))){

		// konstruiere Strahl fuer weitere Messung
		ray r;
		r.dir = rnd_dir();
		r.ori = pos;

		float tmp_dot = vecf4::dot(n,r.dir);
		// Strahl muss nach aussen zeigen
		if (tmp_dot < 0.0f){
			tmp_dot = -tmp_dot;
			r.dir = -r.dir;
		}
		// verhindere Schnittpunkt mit sich selbst
		r.ori += ray_eps * r.dir;

		// berechne skalierte einfallende Strahldichte durch Path Tracing
		const vecf4 tmp = tmp_dot * path_trace(r);

		// schaetze fuer Sampleanzahl Mittelwert und Varianz
		sum += tmp;
		sum_sq += tmp * tmp;
		count++;
		const float inv_count = 1.0f / float(count);
		const float inv_count1 = 1.0f / float(count-1);
		const vecf4 mean = sum * inv_count;
		const vecf4 var = ((sum_sq * inv_count1) - (float(count)*inv_count1 * mean * mean)) * inv_count;
		// berechne den relativen Fehler
		rel_err = max(sqrt(var) / (mean + rel_err_eps));
	}

	// setze zurueckgegebene Sampleanzahl
	sample_count = count;

	// gebe geschaetzte Irradianz zurueck
	return sum * 2.0f * M_PI / float(count);
}