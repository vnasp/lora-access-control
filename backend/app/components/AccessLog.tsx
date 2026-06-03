"use client";

import { useState } from "react";
import { LogIn, LogOut, SlidersHorizontal } from "lucide-react";
import type { LogRow } from "@/app/types";

type EventFilter = "all" | "ENTRY" | "EXIT";

function toChileTime(utc: string) {
  return new Intl.DateTimeFormat("es-CL", {
    timeZone: "America/Santiago",
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit",
    hour12: false,
  }).format(new Date(utc));
}

function toChileDate(utc: string) {
  return new Intl.DateTimeFormat("es-CL", {
    timeZone: "America/Santiago",
    day: "2-digit",
    month: "2-digit",
    year: "numeric",
  }).format(new Date(utc));
}

export default function AccessLog({
  allLogs,
  eventFilter,
}: {
  allLogs: LogRow[];
  eventFilter: EventFilter;
}) {
  const [parcelFilter, setParcelFilter] = useState<string>("all");
  const [showFilters, setShowFilters] = useState(false);

  const filtered = allLogs.filter((r) => {
    if (parcelFilter !== "all" && String(r.parcel_id) !== parcelFilter) return false;
    if (eventFilter !== "all" && r.event_type !== eventFilter) return false;
    return true;
  });

  return (
    <div className="flex flex-1 flex-col overflow-hidden rounded-2xl bg-surface shadow-lg">
      {/* Cabecera */}
      <div className="flex items-center justify-between border-b border-border px-6 py-4">
        <div>
          <p className="text-sm font-bold uppercase tracking-widest text-text-primary">
            Registro comunitario
          </p>
          <p className="text-xs text-text-muted">
            Registro de ingresos y salidas de todas las parcelas
          </p>
        </div>
        <button
          onClick={() => setShowFilters((v) => !v)}
          className="flex items-center gap-1.5 rounded-lg border border-border px-3 py-1.5 text-xs text-text-secondary transition hover:bg-surface-muted"
        >
          <SlidersHorizontal className="h-3.5 w-3.5" />
          Filtrar
        </button>
      </div>

      {/* Badge filtro activo desde SidePanel */}
      {eventFilter !== "all" && (
        <div className="flex items-center gap-2 border-b border-border bg-surface-muted px-6 py-2">
          <span
            className={`rounded-full px-2.5 py-0.5 text-xs font-medium ${eventFilter === "ENTRY" ? "bg-success-bg text-success" : "bg-warning-bg text-warning"}`}
          >
            {eventFilter === "ENTRY" ? "Solo ingresos" : "Solo salidas"}
          </span>
          <span className="text-xs text-text-muted">
            — filtro aplicado desde el resumen
          </span>
        </div>
      )}

      {/* Filtros (colapsables) */}
      {showFilters && (
        <div className="flex items-center gap-2 border-b border-border px-6 py-3">
          <select
            value={parcelFilter}
            onChange={(e) => setParcelFilter(e.target.value)}
            className="rounded-lg border border-border bg-surface px-3 py-1.5 text-xs text-text-secondary"
          >
            <option value="all">Todas las parcelas</option>
            {Array.from({ length: 15 }, (_, i) => (
              <option key={i + 1} value={String(i + 1)}>
                Parcela {String(i + 1).padStart(2, "0")}
              </option>
            ))}
          </select>
        </div>
      )}

      {/* Tabla con scroll */}
      <div className="flex-1 overflow-y-auto">
        {filtered.length === 0 ? (
          <p className="px-6 py-8 text-sm text-text-muted">
            Sin registros todavía.
          </p>
        ) : (
          <table className="w-full text-sm">
            <thead className="sticky top-0 bg-surface-muted">
              <tr className="text-left text-xs font-semibold uppercase tracking-wide text-text-muted">
                <th className="px-5 py-3">Fecha / Hora</th>
                <th className="px-5 py-3">Parcela</th>
                <th className="px-5 py-3">Tipo</th>
                <th className="px-5 py-3">Código</th>
                <th className="px-5 py-3">Estado</th>
                <th className="px-5 py-3">Origen</th>
              </tr>
            </thead>
            <tbody>
              {filtered.map((row) => (
                <tr
                  key={row.id}
                  className="border-t border-border transition-colors hover:bg-surface-muted"
                >
                  <td
                    className="px-5 py-3 font-mono text-xs"
                    suppressHydrationWarning
                  >
                    <span className="block text-text-secondary">
                      {toChileDate(row.timestamp)}
                    </span>
                    <span className="block text-text-secondary">
                      {toChileTime(row.timestamp)}
                    </span>
                  </td>
                  <td className="px-5 py-3 font-semibold text-text-primary">
                    {row.parcel_id
                      ? String(row.parcel_id).padStart(2, "0")
                      : "—"}
                  </td>
                  <td className="px-5 py-3">
                    {row.event_type === "ENTRY" && (
                      <span className="flex items-center gap-1.5 font-medium text-success">
                        <LogIn className="h-4 w-4" /> Ingreso
                      </span>
                    )}
                    {row.event_type === "EXIT" && (
                      <span className="flex items-center gap-1.5 font-medium text-warning">
                        <LogOut className="h-4 w-4" /> Salida
                      </span>
                    )}
                    {!row.event_type && (
                      <span className="text-text-muted">—</span>
                    )}
                  </td>
                  <td className="px-5 py-3 font-mono text-text-secondary">
                    {row.code}
                  </td>
                  <td className="px-5 py-3">
                    <span className="rounded-full bg-success-bg px-2.5 py-0.5 text-xs font-medium text-success">
                      Autorizado
                    </span>
                  </td>
                  <td className="px-5 py-3 text-xs text-text-muted">
                    Nodo Portón
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        )}
      </div>

      {/* Footer */}
      <div className="flex items-center justify-between border-t border-border px-6 py-3">
        <p className="text-xs text-text-muted">
          Mostrando {filtered.length} de {allLogs.length} eventos
        </p>
        <div className="flex items-center gap-1">
          <button className="rounded-lg border border-border px-2.5 py-1 text-xs text-text-muted">
            ‹
          </button>
          <button className="rounded-lg bg-primary px-2.5 py-1 text-xs font-semibold text-white">
            1
          </button>
          <button className="rounded-lg border border-border px-2.5 py-1 text-xs text-text-muted">
            ›
          </button>
        </div>
      </div>
    </div>
  );
}
