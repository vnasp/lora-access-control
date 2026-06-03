"use client";

import { useState } from "react";
import type { LogRow } from "@/app/types";
import SidePanel from "./SidePanel";
import AccessLog from "./AccessLog";

export type { LogRow };

type EventFilter = "all" | "ENTRY" | "EXIT";

type Props = {
  parcelId: number;
  dailyCode: string | null;
  allLogs: LogRow[];
  entries: number;
  exits: number;
  activeParcels: number;
  lastEvent: string;
};

export default function Dashboard({
  parcelId,
  dailyCode,
  allLogs,
  entries,
  exits,
  activeParcels,
  lastEvent,
}: Props) {
  const [eventFilter, setEventFilter] = useState<EventFilter>("all");

  function handleEventFilter(type: "ENTRY" | "EXIT") {
    setEventFilter((prev) => (prev === type ? "all" : type));
  }

  return (
    <div className="relative z-10 mx-auto flex w-full max-w-7xl flex-1 flex-col py-8">
      <h1 className="mb-8 text-4xl font-bold text-text-primary">
        Bienvenida, Parcela {String(parcelId).padStart(2, "0")}
      </h1>

      <div className="flex flex-1 items-stretch gap-6">
        <SidePanel
          dailyCode={dailyCode}
          entries={entries}
          exits={exits}
          activeParcels={activeParcels}
          lastEvent={lastEvent}
          activeEventFilter={eventFilter}
          onEventFilter={handleEventFilter}
        />
        <AccessLog allLogs={allLogs} eventFilter={eventFilter} />
      </div>
    </div>
  );
}
