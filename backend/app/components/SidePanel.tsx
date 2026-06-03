import { Clock, Leaf, LogIn, LogOut, ShieldCheck, Users } from "lucide-react";
import CopyButton from "./CopyButton";

type EventFilter = "all" | "ENTRY" | "EXIT";

type Props = {
  dailyCode: string | null;
  entries: number;
  exits: number;
  activeParcels: number;
  lastEvent: string;
  activeEventFilter: EventFilter;
  onEventFilter: (type: "ENTRY" | "EXIT") => void;
};

export default function SidePanel({
  dailyCode,
  entries,
  exits,
  activeParcels,
  lastEvent,
  activeEventFilter,
  onEventFilter,
}: Props) {
  return (
    <div className="w-[38%] shrink-0 space-y-8">
      {/* Card código */}
      <div className="rounded-2xl bg-[url('/code.png')] bg-no-repeat bg-center bg-size-[110%] space-y-10 p-6 bg-surface-muted shadow shadow-shadow-medium">
        <div className="text-center text-xs font-semibold uppercase tracking-widest text-white">
          Tu código de acceso hoy
        </div>
        <div className="flex flex-row items-center justify-center gap-6">
          <div className="text-6xl font-bold tracking-widest text-white">
            {dailyCode ?? "000000"}
          </div>
          {dailyCode && <CopyButton code={dailyCode} />}
        </div>
        <div className="mx-auto w-1/2">
          <div className="mt-4 flex items-center gap-1.5 rounded-2xl bg-moss/50 px-4 py-2 text-xs text-white">
            <ShieldCheck className="h-4 w-4 shrink-0" />
            Válido hasta las 23:59 hrs
          </div>
        </div>
      </div>

      {/* Nota */}
      <div className="flex items-center gap-4 rounded-xl bg-card px-4 py-4 text-xs text-text-primary shadow shadow-shadow-medium">
        <Leaf className="h-5 w-5" />
        <div className="space-y-2">
          <p>Este código es personal e intransferible.</p>
          <p>Compártelo solo con personas autorizadas.</p>
        </div>
      </div>

      {/* Resumen de hoy */}
      <div className="rounded-2xl bg-card p-4 shadow shadow-shadow-medium">
        <p className="mb-3 text-xs font-bold uppercase tracking-widest text-text-secondary">
          Resumen de hoy
        </p>
        <div className="grid grid-cols-2 gap-3">
          {/* Ingresos — filtrable */}
          <button
            onClick={() => onEventFilter("ENTRY")}
            className={`flex items-center gap-2 rounded-xl border p-3 text-left transition ${activeEventFilter === "ENTRY" ? "border-success bg-success-bg" : "border-border bg-surface-muted/60 hover:bg-surface-muted"}`}
          >
            <LogIn className={`h-5 w-5 shrink-0 ${activeEventFilter === "ENTRY" ? "text-success" : "text-text-secondary"}`} />
            <div>
              <p className="text-lg font-bold leading-none text-text-primary">{entries}</p>
              <p className="text-xs text-text-muted">Ingresos</p>
            </div>
          </button>

          {/* Salidas — filtrable */}
          <button
            onClick={() => onEventFilter("EXIT")}
            className={`flex items-center gap-2 rounded-xl border p-3 text-left transition ${activeEventFilter === "EXIT" ? "border-warning bg-warning-bg" : "border-border bg-surface-muted/60 hover:bg-surface-muted"}`}
          >
            <LogOut className={`h-5 w-5 shrink-0 ${activeEventFilter === "EXIT" ? "text-warning" : "text-text-secondary"}`} />
            <div>
              <p className="text-lg font-bold leading-none text-text-primary">{exits}</p>
              <p className="text-xs text-text-muted">Salidas</p>
            </div>
          </button>

          {/* Parcelas activas */}
          <div className="flex items-center gap-2 rounded-xl border border-border bg-surface-muted/60 p-3">
            <Users className="h-5 w-5 shrink-0 text-text-secondary" />
            <div>
              <p className="text-lg font-bold leading-none text-text-primary">{activeParcels}</p>
              <p className="text-xs text-text-muted">Parcelas activas</p>
            </div>
          </div>

          {/* Último evento */}
          <div className="flex items-center gap-2 rounded-xl border border-border bg-surface-muted/60 p-3">
            <Clock className="h-5 w-5 shrink-0 text-text-secondary" />
            <div>
              <p className="text-lg font-bold leading-none text-text-primary">{lastEvent}</p>
              <p className="text-xs text-text-muted">Último evento</p>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
